#include "adexchange.hpp"


void adexchange::getimp(uint64_t uuid, string ifa,
                        string device_id,
                        string ip, string ua, uint32_t adtype,
                        uint32_t width, uint32_t height, uint32_t pos,
                        string pub, string app) {
    account_name from = N(eosvegasjack);

    string openrtb = "{ \
            \"id\": \"1234567893\", \
            \"at\": 2 a";

    action(permission_level{from, N(active)}, N(eosvegasjack),
           N(getrequest), std::make_tuple(_self, uuid, string(openrtb)))
            .send();
}

void adexchange::receivebid(account_name from, uint64_t uuid, uint64_t price, string adinfo) {

    auto newbidid = bids.available_primary_key();
    bids.emplace(_self, [&](auto& p){
        p.bidid = newbidid;
        p.price = price;
        p.partner = from;
        p.uuid = uuid;
    });

    auto itr_win = wins.find(uuid);
    if (itr_win == wins.end()) {
        wins.emplace(_self, [&](auto& p){
            p.uuid = uuid;
            p.adurl = adinfo;
            p.bidid = newbidid;
            p.created_at = now();

        });
    }
}

void adexchange::clear() {
    auto itr = wins.begin();
    while (itr != wins.end()) {
        itr = wins.erase(itr);
    }
    auto itr2 = bids.begin();
    while (itr2 != bids.end()) {
        itr2 = bids.erase(itr2);
    }
    auto itr3 = reports.begin();
    while (itr3 != reports.end()) {
        itr3 = reports.erase(itr3);
    }
}

void adexchange::winnotice(uint64_t bidid) {
    auto itr_bid = bids.find(bidid);

    auto itr_win = wins.find(itr_bid->uuid);
    uint64_t  h = (itr_win->created_at - 1536422400) / 3600;
    print("======", h);

    auto itr_report = reports.find(h);
    if (itr_report == reports.end()) {
        itr_report = reports.emplace(_self, [&](auto &p) {
            p.partner = itr_bid->partner;
            p.hour = h;
            p.bids = 0;
            p.wins = 0;
            p.clicks = 0;
            p.spend = 0;
        });
    }
    reports.modify(itr_report, _self, [&](auto& p){
        p.wins = p.wins + 1;
        p.spend = p.spend + itr_bid->price;
    });

    wins.modify(itr_win, _self, [&](auto &p) {
        p.winner = itr_bid->partner;
        p.price = itr_bid->price;
        p.bidid = bidid;
    });
}

void adexchange::clicknotice(uint64_t bidid) {
    auto itr_bid = bids.find(bidid);

    auto itr_win = wins.find(itr_bid->uuid);

    wins.modify(itr_win, _self, [&](auto &p) {
        p.hasclick = 1;
    });
        uint64_t  h = (itr_win->created_at - 1536422400) / 3600;
        auto itr_report = reports.find(h);
        if (itr_report == reports.end()) {
            itr_report = reports.emplace(_self, [&](auto &p) {
                p.partner = itr_win->winner;
                p.hour = h;
                p.bids = 0;
                p.wins = 0;
                p.clicks = 0;
                p.spend = 0;
            });
        }
        reports.modify(itr_report, _self, [&](auto& p){
            p.clicks = p.clicks + 1;
        });
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

EOSIO_ABI_EX(adexchange, (getimp)(receivebid)(clear)(winnotice)(clicknotice))