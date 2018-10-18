//
// Created by Kevin Huang on 7/20/18.
//

#ifndef PROJECT_POKERGAME1_H
#define PROJECT_POKERGAME1_H
#endif

//#include <math.h>
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/currency.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/time.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/privileged.h>
#include <set>
#include <sstream>
#include <unordered_map>
#include "eosio.token/eosio.token.hpp"




using namespace eosio;
using std::sort;
using std::string;
using std::stringstream;
using std::hash;
using std::unordered_map;
using std::make_pair;
using std::to_string;


class pokergame1 : public eosio::contract {

public:
    pokergame1(account_name self)
            :contract(self),
             pools(_self, _self),
             events(_self, _self),
             metadatas(_self, _self),
             secrets(_self, _self),
             ginfos(_self, _self),
             gaccounts(_self, _self),
             cardstats(_self, _self),
             typestats(_self, _self),
             paccounts(_self, _self),
             pool5xs(_self, _self),
             suaccounts(_self, _self),
             blacklists(_self, _self), bjpools(_self, _self){};

    //@abi action
    void dealreceipt(const name from, string game, string hash1, string hash2, string cards, string result, string betineos, string winineos);

    //@abi action
    void receipt5x(const name from, string game, string hash1, string hash2, string cards1, string cards2, string cards3,
            string cards4, string cards5, string results, string betineos, string winineo);

    //@abi action
    void drawcards(const name from, uint32_t externalsrc, string dump1, string dump2, string dump3, string dump4, string dump5);
    //@abi action
    void drawcards5x(const name from, uint32_t externalsrc, string dump1, string dump2, string dump3, string dump4, string dump5);
/*
    //@abi action
    void clear();
        */
    //@abi action
    void ramclean();

    /*
    //@abi action
    void setcards(const name from, uint32_t c1, uint32_t c2, uint32_t c3, uint32_t c4, uint32_t c5);
     */
    //@abi action
    void myeosvegas(name vip, string message);
    //@abi action
    void setseed(const name from, uint32_t seed);
    //@abi action
    void setgameon(uint64_t id, uint32_t flag);
    //@abi action
    void setminingon(uint64_t id, uint32_t flag);
    //@abi action
    void getbonus(const name from, const uint32_t type, uint32_t externalsrc);
    //@abi action
    void signup(const name from, const string memo);
    //@abi action
    void blacklist(const name to, uint32_t status);

    checksum256 gethash(account_name from, uint32_t externalsrc, uint32_t rounds);
    void getcards(account_name from, checksum256 result, uint32_t* cards, uint32_t num, std::set<uint32_t> myset, uint64_t hack);
    void deposit(const currency::transfer &t, account_name code, uint32_t bettype);
    bool checkflush(uint32_t colors[5]);
    bool checkstraight(uint32_t numbers[5]);
    uint32_t checksame(uint32_t numbers[5], uint32_t threshold);
    bool ishold(string s);
    bool checkBiggerJack(uint32_t numbers[5]);
    uint32_t parsecard(string s);
    void report(name from, uint64_t minemev, uint64_t meosin, uint64_t meosout);
    uint32_t checkwin(uint32_t c1, uint32_t c2, uint32_t c3, uint32_t c4, uint32_t c5);

private:
    // 0: jacks or better
    // @abi table metadatas i64
    struct st_metadatas {
        uint64_t id;
        uint32_t eventcnt;
        uint32_t idx;   // selection id for random seed
        uint32_t gameon;
        uint32_t miningon;
        uint64_t tmevout;
        uint64_t teosin;
        uint64_t teosout;
        uint64_t trounds;
        uint64_t primary_key() const { return id; }

        EOSLIB_SERIALIZE(st_metadatas, (id)(eventcnt)(idx)(gameon)(miningon)(tmevout)(teosin)(teosout)(trounds))
    };

    // @abi table pools i64
    struct st_pools {
        name owner;
        name referrer;
        uint32_t status;
        uint32_t card1;
        uint32_t card2;
        uint32_t card3;
        uint32_t card4;
        uint32_t card5;
        uint32_t wintype;
        uint32_t betcurrency;
        uint64_t bet;
        uint64_t betwin;
        uint64_t userseed;
        string cardhash1;
        string cardhash2;

        uint64_t primary_key() const { return owner; }

        EOSLIB_SERIALIZE(st_pools, (owner)(referrer)(status)(card1)(card2)(card3)(card4)(card5)(wintype)(betcurrency)(bet)(betwin)(userseed)(cardhash1)(cardhash2))
    };

    // @abi table pool5xs i64
    struct st_pool5xs {
        name owner;
        uint64_t cards1;
        uint64_t cards2;
        uint64_t cards3;
        uint64_t cards4;
        uint64_t cards5;
        uint64_t wintype;
        uint64_t betwin1;
        uint64_t betwin2;
        uint64_t betwin3;
        uint64_t betwin4;
        uint64_t betwin5;

        uint64_t primary_key() const { return owner; }

        EOSLIB_SERIALIZE(st_pool5xs, (owner)(cards1)(cards2)(cards3)(cards4)(cards5)(wintype)(betwin1)(betwin2)(betwin3)(betwin4)(betwin5))
    };


    // @abi table paccounts i64
    struct st_paccounts {
        name owner;
        uint32_t level;
        uint64_t exp;
        uint32_t lastbonus;
        uint32_t lastseen;
        uint32_t logins;    // continuous login days
        uint64_t bonusnumber;
        uint64_t bonustype;

        uint64_t primary_key() const { return owner; }

        EOSLIB_SERIALIZE(st_paccounts, (owner)(level)(exp)(lastbonus)(lastseen)(logins)(bonusnumber)(bonustype))
    };

    // @abi table suaccounts i64
    struct st_suaccounts {
        name owner;

        uint64_t primary_key() const { return owner; }

        EOSLIB_SERIALIZE(st_suaccounts, (owner))
    };

    struct st_blacklists {
        name owner;

        uint64_t primary_key() const { return owner; }

        EOSLIB_SERIALIZE(st_blacklists, (owner))
    };

    // @abi table events i64
    struct st_events {
        uint64_t id;
        name owner;
        uint32_t datetime;
        uint32_t wintype;
        uint32_t ratio;
        uint32_t bet;
        uint32_t betwin;
        uint32_t card1;
        uint32_t card2;
        uint32_t card3;
        uint32_t card4;
        uint32_t card5;

        uint64_t primary_key() const { return id; }

        EOSLIB_SERIALIZE(st_events, (id)(owner)(datetime)(wintype)(ratio)(bet)(betwin)(card1)(card2)(card3)(card4)(card5))
    };

    struct st_secrets {
        uint64_t id;
        uint64_t s1;
        uint64_t primary_key() const { return id; }
        EOSLIB_SERIALIZE(st_secrets, (id)(s1))
    };

    // @abi table ginfos i64
    struct st_ginfos {
        uint32_t startmonth;
        uint64_t tmevout;
        uint64_t teosin;
        uint64_t teosout;

        uint64_t primary_key() const { return startmonth; }
        EOSLIB_SERIALIZE(st_ginfos, (startmonth)(tmevout)(teosin)(teosout))
    };

    // @abi table gaccounts i64
    struct st_gaccounts {
        name owner;
        uint64_t trounds;
        uint64_t tmevout;
        uint64_t teosin;
        uint64_t teosout;
        uint32_t daystart;
        uint64_t drounds;
        uint64_t dmevout;
        uint64_t deosin;
        uint64_t deosout;
        uint32_t weekstart;
        uint64_t wrounds;
        uint64_t wmevout;
        uint64_t weosin;
        uint64_t weosout;
        uint32_t monthstart;
        uint64_t mrounds;
        uint64_t mmevout;
        uint64_t meosin;
        uint64_t meosout;

        uint64_t primary_key() const { return owner; }

        EOSLIB_SERIALIZE(st_gaccounts, (owner)(trounds)(tmevout)(teosin)(teosout)(daystart)(drounds)(dmevout)(deosin)(deosout)(weekstart)(wrounds)(wmevout)(weosin)(weosout)(monthstart)(mrounds)(mmevout)(meosin)(meosout))
    };

    // @abi table cardstats i64
    struct st_cardstats {
        uint64_t id;
        uint64_t count;
        uint64_t primary_key() const { return id; }
        EOSLIB_SERIALIZE(st_cardstats, (id)(count))
    };

    // @abi table typestats i64
    struct st_typestats {
        uint64_t id;
        uint64_t count;
        uint64_t primary_key() const { return id; }
        EOSLIB_SERIALIZE(st_typestats, (id)(count))
    };

/*
    // @abi table logbonus i64
    struct st_logbonus {
        uint64_t owner;
        uint32_t lastbonus;
        uint64_t primary_key() const { return owner; }
        EOSLIB_SERIALIZE(st_logbonus, (owner)(lastbonus))
    };


*/


    // @abi table bjpools i64
    struct st_bjpools {
        uint64_t owner;
        uint32_t status;
        uint64_t cards1;
        uint64_t cards2;


        uint64_t primary_key() const { return owner; }
        EOSLIB_SERIALIZE(st_bjpools, (owner)(lastbonus))
    };




    typedef multi_index<N(metadatas), st_metadatas> _tb_metadatas;
    _tb_metadatas metadatas;
    typedef multi_index<N(pools), st_pools> _tb_pools;
    _tb_pools pools;
    typedef multi_index<N(pool5xs), st_pool5xs> _tb_pool5xs;
    _tb_pool5xs pool5xs;
    typedef multi_index<N(events), st_events> _tb_events;
    _tb_events events;
    typedef multi_index<N(secrets), st_secrets> _tb_secrets;
    _tb_secrets secrets;
    typedef multi_index<N(ginfos), st_ginfos> _tb_ginfos;
    _tb_ginfos ginfos;
    typedef multi_index<N(gaccounts), st_gaccounts> _tb_gaccounts;
    _tb_gaccounts gaccounts;
    typedef multi_index<N(cardstats), st_cardstats> _tb_cardstats;
    _tb_cardstats cardstats;
    typedef multi_index<N(typestats), st_typestats> _tb_typestats;
    _tb_typestats typestats;
    typedef multi_index<N(paccounts), st_paccounts> _tb_paccounts;
    _tb_paccounts paccounts;

    typedef multi_index<N(suaccounts), st_suaccounts> _tb_suaccounts;
    _tb_suaccounts suaccounts;
    typedef multi_index<N(blacklists), st_blacklists> _tb_blacklists;
    _tb_blacklists blacklists;

    typedef multi_index<N(bjpools), st_bjpools> _tb_bjpools;
    _tb_bjpools bjpools;
};
