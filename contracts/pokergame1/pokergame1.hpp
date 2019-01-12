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

//#include "eos_api.hpp"


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
             ginfos(_self, _self),
             cardstats(_self, _self),
             typestats(_self, _self),
             paccounts(_self, _self),
             pool5xs(_self, _self),
             blacklists(_self, _self),
             bjpools(_self, _self),
             uthpools(_self, _self),
             pevents(_self, _self),
             puevents(_self, _self),
             bjwins(_self, _self),
             partners(_self, _self),
             referrals(_self, _self),
             refouts(_self, _self),
             bjevents(_self, _self),
             jackevents(_self, _self),
             nonces(_self, _self),
             bjnonces(_self, _self),
             vppools(_self, _self),
             uthnonces(_self, _self),
             pubkeys(_self, _self),
             prefs(_self, _self),
             mevouts(_self, _self),
             bjmevouts(_self, _self),
             newyears(_self, _self),
             tnewyears(_self, _self){};

    //@abi action
    void luck(const name player, uint64_t bonus, uint32_t status);

    //@abi action
    void vpreceipt(string game_id, const name player, string game, std::vector<string> player_hand,
                   string bet, string win, string wintype, string seed, string dealer_signature,
                   uint64_t betnum, uint64_t winnum, string token, string pub_key, uint64_t event_win);
    //@abi action
    void vp5xreceipt(string game_id, const name player, string game, std::vector<string> player_hand1
            , std::vector<string> player_hand2, std::vector<string> player_hand3, std::vector<string> player_hand4,
            std::vector<string> player_hand5, string bet, string win, string wintype, string seed,
            string dealer_signature, uint64_t betnum, uint64_t winnum, string token, string pub_key, uint64_t event_win);

    //@abi action
    void clear(account_name owner);
    //@abi action
    void resetdivi();
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
    //@abi action
    void init(string text);

    //@abi action
    void forceclear(const name from);

    //@abi action
    void bjclear(const name from);
    //@abi action
    void uthclear(const name from);

    uint32_t increment_nonce(const name user);
    uint32_t increment_bjnonce(const name user);
    uint32_t increment_uthnonce(const name user);

    //@abi action
    void bjstand(const name player, uint32_t nonce, std::vector<uint32_t> dealer_hand,
                 std::vector<uint32_t> player_hand1, std::vector<uint32_t> player_hand2);
    //@abi action
    void bjhit(const name player, uint32_t nonce, std::vector<uint32_t> dealer_hand,
               std::vector<uint32_t> player_hand1, std::vector<uint32_t> player_hand2);

    //@abi action
    void bjstand1(const name player, uint32_t nonce, std::vector<uint32_t> dealer_hand,
            std::vector<uint32_t> player_hand1);
    //@abi action
    void bjhit1(const name player, uint32_t nonce, std::vector<uint32_t> dealer_hand,
            std::vector<uint32_t> player_hand1);
    //@abi action
    void bjstand2(const name player, uint32_t nonce, std::vector<uint32_t> dealer_hand,
            std::vector<uint32_t> player_hand2);
    //@abi action
    void bjhit2(const name player, uint32_t nonce, std::vector<uint32_t> dealer_hand,
            std::vector<uint32_t> player_hand2);


    //@abi action
    void bjuninsure(const name player, uint32_t nonce, std::vector<uint32_t> dealer_hand,
                    std::vector<uint32_t> player_hand1, std::vector<uint32_t> player_hand2);
    //@abi action
    void bjreceipt(string game_id, const name player, string game, string seed,  std::vector<string> dealer_hand,
                   std::vector<string> player_hand1, std::vector<string> player_hand2, string bet, string win,
                   string insure_bet, string insure_win, uint64_t betnum, uint64_t winnum, uint64_t insurance,
                   uint64_t insurance_win, string token, string actions, string pub_key);


    //@abi action
    void uthcheck(const name player, uint32_t nonce, std::vector<uint32_t> common_hand, std::vector<uint32_t> player_hand, uint32_t cur_status);
    //@abi action
    void uthfold(const name player, uint32_t nonce, std::vector<uint32_t> common_hand, std::vector<uint32_t> player_hand);
    //@abi action
    void uthreceipt(string game_id, const name player, string game, string seed, std::vector<string> dealer_hand,
            std::vector<string> common_hand, std::vector<string> player_hand, string ante, string blind, string trip,
            string play, string ante_win, string blind_win, string trip_win, string play_win, string dealer_win_type,
            string player_win_type, uint64_t betnum, uint64_t winnum, string token, string actions, string pub_key);

    //@abi action
    void addpartner(const account_name partner, uint32_t rate);

    checksum256 gethash(account_name from, uint32_t externalsrc, uint32_t rounds);
    void getcards(account_name from, checksum256 result, uint32_t* cards, uint32_t num, std::set<uint32_t> myset, uint64_t hack, uint32_t maxcard);
    void deposit(const currency::transfer &t, account_name code, uint32_t bettype);
    bool checkflush(uint32_t colors[5]);
    bool checkstraight(uint32_t numbers[5]);
    uint32_t checksame(uint32_t numbers[5], uint32_t threshold);
    bool ishold(string s);
    bool checkBiggerJack(uint32_t numbers[5]);
    uint32_t parsecard(string s);
    void report(name from, uint64_t minemev, uint64_t meosin, uint64_t meosout);
    uint32_t checkwin(uint32_t c1, uint32_t c2, uint32_t c3, uint32_t c4, uint32_t c5);

    uint32_t checkace(uint32_t numbers[5]);

    void depositg2(const currency::transfer &t, uint32_t gameid, uint32_t trounds);
    void bj_get_cards(uint64_t cards, uint32_t count, uint32_t* arr);
    uint32_t bj_get_stat(uint32_t status);
/*
    // @abi action
    void callback( checksum256 queryId, std::vector<unsigned char> result, std::vector<unsigned char> proof );
*/

    void payref(name from, uint64_t bet, uint32_t defaultrate, uint32_t multiplier);

    //@abi action
    void vpdraw(const name from, uint32_t nonce, std::vector<string> actions);

    //@abi action
    void setpubkey(string public_key);

    void updatemevout(name player, uint32_t nonce, uint64_t mevout, uint32_t mode);

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


    // @abi table nonces i64
    struct st_nonces {
        name owner;
        uint32_t number;

        uint64_t primary_key() const { return owner; }
        EOSLIB_SERIALIZE(st_nonces, (owner)(number))
    };

    // @abi table bjnonces i64
    struct st_bjnonces {
        name owner;
        uint32_t number;

        uint64_t primary_key() const { return owner; }
        EOSLIB_SERIALIZE(st_bjnonces, (owner)(number))
    };

    // @abi table uthnonces i64
    struct st_uthnonces {
        name owner;
        uint32_t number;

        uint64_t primary_key() const { return owner; }
        EOSLIB_SERIALIZE(st_uthnonces, (owner)(number))
    };

    // @abi table vppools i64
    struct st_vppools {
        name owner;
        uint32_t status;
        uint32_t mode;
        uint64_t nonce;
        uint64_t bet;
        string card1;
        string card2;
        string card3;
        string card4;
        string card5;
        string bettoken;
        string seed;

        uint64_t primary_key() const { return owner; }
        EOSLIB_SERIALIZE(st_vppools, (owner)(status)(mode)(nonce)(bet)(card1)(card2)(card3)(card4)(card5)(bettoken)(seed))
    };

    // @abi table mevouts i64
    struct st_mevouts {
        name owner;
        uint32_t nonce;
        uint64_t mevout;
        uint64_t lastseen;

        uint64_t primary_key() const { return owner; }
        EOSLIB_SERIALIZE(st_mevouts, (owner)(nonce)(mevout)(lastseen))
    };

    // @abi table bjmevouts i64
    struct st_bjmevouts {
        name owner;
        uint32_t nonce;
        uint64_t mevout;
        uint64_t lastseen;

        uint64_t primary_key() const { return owner; }
        EOSLIB_SERIALIZE(st_bjmevouts, (owner)(nonce)(mevout)(lastseen))
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


    // @abi table bjpools i64
    struct st_bjpools {
        name owner;
        uint32_t status;
        uint64_t nonce;
        uint64_t dcards;
        uint32_t dcnt;
        uint64_t pcards1;
        uint32_t pcnt1;
        uint64_t pcards2;
        uint32_t pcnt2;
        uint64_t bet;
        uint64_t insurance;
        string bettoken;
        string seed;
        string actions;

        uint64_t primary_key() const { return owner; }

        EOSLIB_SERIALIZE(st_bjpools, (owner)(status)(nonce)(dcards)(dcnt)(pcards1)(pcnt1)(pcards2)(pcnt2)(bet)(insurance)(bettoken)(seed)(actions))
    };


    // @abi table uthpools i64
    struct st_uthpools {
        name owner;
        uint32_t status;
        uint64_t nonce;
        uint64_t dcards;
        uint64_t pcards;
        uint64_t bet;
        uint64_t trip;
        uint64_t ante;
        uint64_t play;
        string bettoken;
        string seed;
        string actions;

        uint64_t primary_key() const { return owner; }

        EOSLIB_SERIALIZE(st_uthpools, (owner)(status)(nonce)(dcards)(pcards)(bet)(trip)(ante)(play)(bettoken)(seed)(actions))
    };

    // @abi table bjwins i64
    struct st_bjwins {
        uint64_t id;
        name owner;
        uint32_t datetime;
        uint32_t win;

        uint64_t primary_key() const { return id; }

        EOSLIB_SERIALIZE(st_bjwins, (id)(owner)(datetime)(win))
    };

    // @abi table pubkeys i64
    struct st_pubkeys {
        uint64_t id;
        string pubkey;
        uint32_t updatetime;

        uint64_t primary_key() const { return id; }
        EOSLIB_SERIALIZE(st_pubkeys, (id)(pubkey)(updatetime))
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

    // @abi table ginfos i64
    struct st_ginfos {
        uint32_t startmonth;
        uint64_t tmevout;
        uint64_t teosin;
        uint64_t teosout;

        uint64_t primary_key() const { return startmonth; }
        EOSLIB_SERIALIZE(st_ginfos, (startmonth)(tmevout)(teosin)(teosout))
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

    // @abi table pevents i64
    struct st_pevents {
        uint64_t id;
        uint64_t count;
        uint64_t eosin;

        uint64_t primary_key() const { return id; }
        EOSLIB_SERIALIZE(st_pevents, (id)(count)(eosin))
    };


    // @abi table puevents i64
    struct st_puevents {
        uint64_t id;
        name owner;
        uint64_t type;

        uint64_t primary_key() const { return id; }
        EOSLIB_SERIALIZE(st_puevents, (id)(owner)(type))
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

    // @abi table partners i64
    struct st_partners {
        name owner;
        uint32_t rate;
        uint64_t eosout;
        uint64_t count;
        uint64_t balance;

        uint64_t primary_key() const { return owner; }
        EOSLIB_SERIALIZE(st_partners, (owner)(rate)(eosout)(count)(balance))
    };

    // @abi table refouts i64
    struct st_refouts {
        uint64_t id;
        uint64_t eosout;
        uint64_t count;

        uint64_t primary_key() const { return id; }
        EOSLIB_SERIALIZE(st_refouts, (id)(eosout)(count))
    };

    // @abi table bjevents i64
    struct st_bjevents {
        name owner;
        uint64_t count;
        uint64_t eosout;

        uint64_t primary_key() const { return owner; }
        EOSLIB_SERIALIZE(st_bjevents, (owner)(count)(eosout))
    };

    // @abi table jackevents i64
    struct st_jackevents {
        name owner;
        uint64_t eosin;

        uint64_t primary_key() const { return owner; }
        EOSLIB_SERIALIZE(st_jackevents, (owner)(eosin))
    };

    // @abi table referrals i64
    struct st_referrals {
        name owner;
        name referrer;

        uint64_t primary_key() const { return owner; }
        EOSLIB_SERIALIZE(st_referrals, (owner)(referrer))
    };

    // @abi table prefs i64
    struct st_prefs {
        name owner;
        name partner;

        uint64_t primary_key() const { return owner; }
        EOSLIB_SERIALIZE(st_prefs, (owner)(partner))
    };

    // @abi table newyears i64
    struct st_newyears {
        name owner;
        uint64_t eosin;
        uint32_t status;

        uint64_t primary_key() const { return owner; }
        EOSLIB_SERIALIZE(st_newyears, (owner)(eosin)(status))
    };
    // @abi table tnewyears i64
    struct st_tnewyears {
        uint64_t id;
        uint64_t eosin;

        uint64_t primary_key() const { return id; }
        EOSLIB_SERIALIZE(st_tnewyears, (id)(eosin))
    };


    typedef multi_index<N(metadatas), st_metadatas> _tb_metadatas;
    _tb_metadatas metadatas;
    typedef multi_index<N(pools), st_pools> _tb_pools;
    _tb_pools pools;
    typedef multi_index<N(pool5xs), st_pool5xs> _tb_pool5xs;
    _tb_pool5xs pool5xs;
    typedef multi_index<N(events), st_events> _tb_events;
    _tb_events events;
    typedef multi_index<N(ginfos), st_ginfos> _tb_ginfos;
    _tb_ginfos ginfos;
    typedef multi_index<N(cardstats), st_cardstats> _tb_cardstats;
    _tb_cardstats cardstats;
    typedef multi_index<N(typestats), st_typestats> _tb_typestats;
    _tb_typestats typestats;
    typedef multi_index<N(paccounts), st_paccounts> _tb_paccounts;
    _tb_paccounts paccounts;

    typedef multi_index<N(blacklists), st_blacklists> _tb_blacklists;
    _tb_blacklists blacklists;

    typedef multi_index<N(bjpools), st_bjpools> _tb_bjpools;
    _tb_bjpools bjpools;
    typedef multi_index<N(bjwins), st_bjwins> _tb_bjwins;
    _tb_bjwins bjwins;

    typedef multi_index<N(uthpools), st_uthpools> _tb_uthpools;
    _tb_uthpools uthpools;

    typedef multi_index<N(pevents), st_pevents> _tb_pevents;
    _tb_pevents pevents;
    typedef multi_index<N(puevents), st_puevents> _tb_puevents;
    _tb_puevents puevents;

    typedef multi_index<N(partners), st_partners> _tb_partners;
    _tb_partners partners;
    typedef multi_index<N(refouts), st_refouts> _tb_refouts;
    _tb_refouts refouts;

    typedef multi_index<N(referrals), st_referrals> _tb_referrals;
    _tb_referrals referrals;
    typedef multi_index<N(prefs), st_prefs> _tb_prefs;
    _tb_prefs prefs;

    typedef multi_index<N(bjevents), st_bjevents> _tb_bjevents;
    _tb_bjevents bjevents;
    typedef multi_index<N(jackevents), st_jackevents> _tb_jackevents;
    _tb_jackevents jackevents;


    typedef multi_index<N(vppools), st_vppools> _tb_vppools;
    _tb_vppools vppools;
    typedef multi_index<N(nonces), st_nonces> _tb_nonces;
    _tb_nonces nonces;
    typedef multi_index<N(bjnonces), st_bjnonces> _tb_bjnonces;
    _tb_bjnonces bjnonces;
    typedef multi_index<N(uthnonces), st_uthnonces> _tb_uthnonces;
    _tb_uthnonces uthnonces;

    typedef multi_index<N(pubkeys), st_pubkeys> _tb_pubkeys;
    _tb_pubkeys pubkeys;

    typedef multi_index<N(mevouts), st_mevouts> _tb_mevouts;
    _tb_mevouts mevouts;
    typedef multi_index<N(bjmevouts), st_bjmevouts> _tb_bjmevouts;
    _tb_bjmevouts bjmevouts;

    typedef multi_index<N(newyears), st_newyears> _tb_newyears;
    _tb_newyears newyears;
    typedef multi_index<N(tnewyears), st_tnewyears> _tb_tnewyears;
    _tb_tnewyears tnewyears;
};
