#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/currency.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/time.hpp>
#include <eosiolib/transaction.hpp>
#include <set>
#include <sstream>
#include <unordered_map>

using namespace eosio;
using std::sort;
using std::string;
using std::stringstream;
using std::hash;
using std::unordered_map;
using std::make_pair;


class adexchange : public eosio::contract {

public:
    adexchange(account_name self)
            :contract(self),wins(_self, _self),bids(_self, _self),reports(self, _self){};
    //@abi action
    void getimp(uint64_t uuid, string ifa,
                    string device_id,
                    string ip, string ua, uint32_t adtype,
                    uint32_t width, uint32_t height, uint32_t pos,
                    string pub, string app);
    //@abi action
    void receivebid(account_name from, uint64_t uuid, uint64_t price, string adinfo);

    //@abi action
    void clear();

    //@abi action
    void winnotice(uint64_t bidid);
    //@abi action
    void clicknotice(uint64_t bidid);

private:
    // @abi table wins i64
    struct st_wins {
        uint64_t uuid;
        string adurl;
        uint64_t bidid;
        account_name winner;
        uint64_t price;
        uint32_t created_at;
        uint32_t hasclick;

        uint64_t primary_key() const { return uuid; }

        EOSLIB_SERIALIZE(st_wins, (uuid)(adurl)(bidid)(winner)(price)(created_at)(hasclick));
    };

    typedef multi_index<N(wins), st_wins> _tb_wins;
    _tb_wins wins;


    // @abi table bids i64
    struct st_bids {
        uint64_t bidid;
        uint64_t price;
        account_name partner;
        uint64_t uuid;

        uint64_t primary_key() const { return bidid; }

        EOSLIB_SERIALIZE(st_bids, (bidid)(price)(partner)(uuid));
    };
    typedef multi_index<N(bids), st_bids> _tb_bids;
    _tb_bids bids;

    // @abi table reports i64
    struct st_reports {
        account_name partner;
        uint64_t hour;
        uint32_t bids;
        uint32_t wins;
        uint32_t clicks;
        uint64_t spend;

        uint64_t primary_key() const { return hour; }

        EOSLIB_SERIALIZE(st_reports, (partner)(hour)(bids)(wins)(clicks)(spend));
    };
    typedef multi_index<N(reports), st_reports> _tb_reports;
    _tb_reports reports;

};