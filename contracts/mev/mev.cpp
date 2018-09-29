#include "mev.hpp"


uint64_t mstarttimes[4] = {1535760000, 1538352000, 1541030400, 1543622400};


uint32_t getstartmonth(uint32_t epochtime) {
    for (int i = 0; i < 4; i++) {
        if (epochtime <= mstarttimes[i]) {
            return mstarttimes[i-1];
        }
    }
    return mstarttimes[3];
}

void mev::create( account_name issuer,
                    asset        maximum_supply )
{
    require_auth( _self );

    auto sym = maximum_supply.symbol;
    eosio_assert( sym.is_valid(), "invalid symbol name" );
    eosio_assert( maximum_supply.is_valid(), "invalid supply");
    eosio_assert( maximum_supply.amount > 0, "max-supply must be positive");

    stats statstable( _self, sym.name() );
    auto existing = statstable.find( sym.name() );
    eosio_assert( existing == statstable.end(), "token with symbol already exists" );

    statstable.emplace( _self, [&]( auto& s ) {
       s.supply.symbol = maximum_supply.symbol;
       s.max_supply    = maximum_supply;
       s.issuer        = issuer;
    });
}


void mev::issue( account_name to, asset quantity, string memo )
{
    auto sym = quantity.symbol;
    eosio_assert( sym.is_valid(), "invalid symbol name" );
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    auto sym_name = sym.name();
    stats statstable( _self, sym_name );
    auto existing = statstable.find( sym_name );
    eosio_assert( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
    const auto& st = *existing;

    require_auth( st.issuer );
    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must issue positive quantity" );

    eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    eosio_assert( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify( st, 0, [&]( auto& s ) {
       s.supply += quantity;
    });

    add_balance( st.issuer, quantity, st.issuer );

    if( to != st.issuer ) {
       SEND_INLINE_ACTION( *this, transfer, {st.issuer,N(active)}, {st.issuer, to, quantity, memo} );
    }
}

void mev::transfer( account_name from,
                      account_name to,
                      asset        quantity,
                      string       memo )
{
    eosio_assert( from != to, "cannot transfer to self" );
    require_auth( from );
    eosio_assert( is_account( to ), "to account does not exist");
    auto sym = quantity.symbol.name();
    stats statstable( _self, sym );
    const auto& st = statstable.get( sym );

    require_recipient( from );
    require_recipient( to );

    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must transfer positive quantity" );
    eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );


    sub_balance( from, quantity );
    add_balance( to, quantity, from );
}

void mev::sub_balance( account_name owner, asset value ) {
   accounts from_acnts( _self, owner );

   const auto& from = from_acnts.get( value.symbol.name(), "no balance object found" );
   eosio_assert( from.balance.amount >= value.amount, "overdrawn balance!" );

   if( from.balance.amount == value.amount ) {
      from_acnts.erase( from );
      auto itr_holder = holders.find(owner);
      if (itr_holder != holders.end()) {
          holders.erase(itr_holder);
      }
   } else {
      from_acnts.modify( from, owner, [&]( auto& a ) {
          a.balance -= value;
      });
   }
}

void mev::add_balance( account_name owner, asset value, account_name ram_payer )
{
   accounts to_acnts( _self, owner );
   auto to = to_acnts.find( value.symbol.name() );
   if( to == to_acnts.end() ) {
      to_acnts.emplace( ram_payer, [&]( auto& a ){
        a.balance = value;
      });

      auto itr_holder = holders.find(owner);
      if (itr_holder == holders.end()) {
          holders.emplace(_self, [&](auto &p){
              p.owner = owner;
          });
      }

   } else {
      to_acnts.modify( to, 0, [&]( auto& a ) {
        a.balance += value;
      });
   }
}

void mev::debug(account_name from) {
    require_auth( _self );
    accounts from_acnts( _self, from );

    asset bal = asset(0, symbol_type(S(4, MEV)));
    const auto& itr_user = from_acnts.get( bal.symbol.name(), "no balance object found" );

    from_acnts.erase( itr_user );

    int32_t ram_bytes = action_data_size();
    print("======",ram_bytes);
}

void mev::registerbb(account_name from, asset quantity) {
    eosio_assert( is_account( from ), "User does not exist");
    require_auth( from );
    asset tempasset = asset(0, symbol_type(S(4, MEV)));
    stats statstable( _self, tempasset.symbol.name() );
    const auto& st = statstable.get( tempasset.symbol.name() );

    eosio_assert( quantity.symbol.name() == tempasset.symbol.name(), "Symbol must be MEV" );
    eosio_assert( quantity.is_valid(), "Invalid quantity" );
    eosio_assert( quantity.amount > 0, "Must transfer positive quantity" );
    eosio_assert( quantity.symbol == st.supply.symbol, "Symbol precision mismatch" );

    auto amount = quantity.amount;
    uint32_t cur_in_sec = now();
    uint32_t month_in_sec = getstartmonth(cur_in_sec);
    auto itr_rewardinfo = rewardinfos.find(month_in_sec);
    eosio_assert( itr_rewardinfo != rewardinfos.end(), "Repurchase window is not open yet for this month.");
    eosio_assert( cur_in_sec < itr_rewardinfo->expireat , "Repurchase window is closed for this month.");
    rewardinfos.modify( itr_rewardinfo, 0, [&]( auto& p ) {
        p.buybackshares += amount;
    });

    sub_balance( from, quantity );
    add_balance( N(eosvegascoin), quantity, st.issuer );

    auto itr_buyback = buybackregs.find(from);
    eosio_assert( itr_buyback == buybackregs.end(), "You have registered buyback for this month." );
    itr_buyback = buybackregs.emplace(_self, [&](auto &p){
        p.owner = from;
        p.shares = quantity;
    });
}

void mev::cancelreward(account_name from) {
    if (from != N(eosvegasjack) || from != _self) {
        eosio_assert( false, "You don't have privilege to cancel reward." );

    }

    require_auth( _self );
}

void mev::rewardholders(const currency::transfer &t, account_name code) {
    // memo should be like "time"

    if (t.from != N(eosvegasjack)) {
        return;
    }
    eosio_assert(t.quantity.symbol == string_to_symbol(4, "EOS"), "Only accepts EOS/MEV for deposits.");
    eosio_assert(t.to == _self, "Transfer not made to this contract");
    eosio_assert(t.quantity.is_valid(), "Invalid token transfer");
    eosio_assert(t.quantity.amount > 0, "Quantity must be positive");

    auto amount = t.quantity.amount;
    string usercomment = t.memo;
    string ucm = usercomment.substr(0, 10);
    uint32_t startmonth = stoi(ucm);
    auto itr_rewardinfo = rewardinfos.find(startmonth);

    uint64_t bbshares = 0;
    uint64_t bbprice = 0;
    if (amount > itr_rewardinfo->buybackshares) {

    }
    // buyback


}

void addreward(uint32_t month, uint32_t status) {
}

void removereward(uint32_t month) {

}


#define EOSIO_ABI_EX( TYPE, MEMBERS ) \
extern "C" { \
   void apply(uint64_t receiver, uint64_t code, uint64_t action) { \
      auto self = receiver; \
      if( action == N(onerror)) { \
         /* onerror is only valid if it is for the "eosio" code account and authorized by "eosio"'s "active permission */ \
         eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account"); \
      } \
      if(code == self || code == N(eosio.token)) { \
         TYPE thiscontract(self); \
         if (action == N(transfer) && code == N(eosio.token)) { \
              currency::transfer tr = unpack_action_data<currency::transfer>(); \
              if (tr.to == self) { \
                  thiscontract.rewardholders(tr, code); \
              } \
              return; \
         } \
         if (code != self) { \
             return; \
         } \
         switch(action) { \
            EOSIO_API(TYPE, MEMBERS) \
         } \
      } \
   } \
}

EOSIO_ABI_EX( mev, (create)(issue)(transfer)(debug)(registerbb)(rewardholders) )
