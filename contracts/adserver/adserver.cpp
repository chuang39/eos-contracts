#include "adserver.hpp"


void adserver::getrequest(account_name from, uint64_t uuid) {
    print("========sendrequest");

    uint64_t price = ((tapos_block_num() % 100) + 50) % 100;

    action(permission_level{from, N(active)}, N(blockfishbgp),
           N(receivebid), std::make_tuple(_self, uuid, price, string("https://www.blockfish.io/LBXRes/adinfo001.js")))
            .send();

}


#define EOSIO_ABI_EX( TYPE, MEMBERS ) \
extern "C" { \
   void apply(uint64_t receiver, uint64_t code, uint64_t action) { \
      auto self = receiver; \
      if( action == N(onerror)) { \
         /* onerror is only valid if it is for the "eosio" code account and authorized by "eosio"'s "active permission */ \
         eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account"); \
      } \
      if(code == self || code == N(eosio.token) || action == N(onerror)) { \
         TYPE thiscontract(self); \
         if (action == N(transfer)) { \
             currency::transfer tr = unpack_action_data<currency::transfer>(); \
             if (tr.to == self) { \
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

EOSIO_ABI_EX(adserver, (getrequest))