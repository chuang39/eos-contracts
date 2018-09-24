#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/currency.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/time.hpp>

#include <string>

using namespace eosio;
using std::string;

class mev : public contract {
  public:
     mev( account_name self ):contract(self),buybackregs(_self, _self){};
     //@abi action
     void create( account_name issuer,
                  asset        maximum_supply);
    //@abi action
     void issue( account_name to, asset quantity, string memo );
    //@abi action
     void transfer( account_name from,
                    account_name to,
                    asset        quantity,
                    string       memo );
    //@abi action
    void debug(account_name from);
    //@abi action
    void registerbb(account_name from, asset quantity);

    void paydividend(const currency::transfer &t, account_name code);
     inline asset get_supply( symbol_name sym )const;

     inline asset get_balance( account_name owner, symbol_name sym )const;

  private:
     //@abi table accounts i64
     struct account {
        asset    balance;

        uint64_t primary_key()const { return balance.symbol.name(); }
     };

     //@abi table stat i64
     struct statt {
        asset          supply;
        asset          max_supply;
        account_name   issuer;

        uint64_t primary_key()const { return supply.symbol.name(); }
     };

     struct st_buybackregs {
         uint64_t id;
         account_name owner;
         asset shares;

         uint64_t get_buyback_by_owner() const {return owner;}

         uint64_t primary_key()const { return id; }
         EOSLIB_SERIALIZE(st_buybackregs, (id)(owner)(shares))
     };
     typedef multi_index<N(buybackregs), st_buybackregs,
            indexed_by<N(byowner), const_mem_fun<st_buybackregs, uint64_t, &st_buybackregs::get_buyback_by_owner>>
     > _tb_buybackregs;
     _tb_buybackregs buybackregs;


     typedef multi_index<N(accounts), account> accounts;
     typedef multi_index<N(stat), statt> stats;

     void sub_balance( account_name owner, asset value );
     void add_balance( account_name owner, asset value, account_name ram_payer );

  public:
     struct transfer_args {
        account_name  from;
        account_name  to;
        asset         quantity;
        string        memo;
     };
};

asset mev::get_supply( symbol_name sym )const
{
  stats statstable( _self, sym );
  const auto& st = statstable.get( sym );
  return st.supply;
}

asset mev::get_balance( account_name owner, symbol_name sym )const
{
  accounts accountstable( _self, owner );
  const auto& ac = accountstable.get( sym );
  return ac.balance;
}

