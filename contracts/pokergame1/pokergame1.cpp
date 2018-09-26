#include "pokergame1.hpp"

#define DEBUG 0
// What's this table used for? God knows!

// TODO; prevent malicious ram stealing
//TODO: fix mining table
// TODO: check if now() is UTC time or local time
// eos * miningtable[][1] / 100
uint64_t miningtable[5][2] = {{4000000, 400}, // 1EOS 4MEV
                              {4000000 * 2, 200}, // 1EOS 2MEV
                               {4000000 * 3, 100}, // 1EOS 1MEV
                               {4000000 * 4, 50}, // 2EOS 1MEV
                               {4000000 * 5, 10} // 10EOS 1MEV
                              };

const uint32_t starttime = 1537833600; // 18/09/25 00:00:00 UTC
const uint32_t wstarttime = 1537747200; //GMT: Monday, September 24, 2018 12:00:00 AM
uint64_t mstarttimes[4] = {1535760000, 1538352000, 1541030400, 1543622400};

uint64_t getminingtableprice(uint64_t sold_keys) {
    for (int i = 0; i < 5; i++) {
        if (sold_keys < miningtable[i][0]) {
            return miningtable[i][1];
        }
    }
    return 10;
}


uint32_t getstartweek(uint32_t epochtime) {
    return epochtime - ((epochtime - wstarttime) % 25200);
}

uint32_t getstartday(uint32_t epochtime) {
    return epochtime - ((epochtime - starttime) % 3600);
}

uint32_t getstartmonth(uint32_t epochtime) {
    for (int i = 0; i < 4; i++) {
        if (epochtime <= mstarttimes[i]) {
            return mstarttimes[i-1];
        }
    }
    return mstarttimes[3];
}

checksum256 pokergame1::gethash(account_name from) {
    auto itr_metadata = metadatas.begin();
    auto itr_secret = secrets.find(itr_metadata->idx);
    uint64_t seed = itr_secret->s1;

    checksum256 result;
    int bnum = tapos_block_num();
    sha256((char *)&seed, sizeof(seed), &result);


    secrets.modify(itr_secret, _self, [&](auto &p){
        p.s1 = bnum + current_time() + from;
    });
    metadatas.modify(itr_metadata, _self, [&](auto &p){
        p.idx = (p.idx + 1) % 256;
    });

    return result;
}

uint32_t pokergame1::getcard(account_name from, checksum256 result) {
    uint64_t seed = result.hash[1];
    seed <<= 32;
    seed |= result.hash[0];

    uint32_t res = (uint32_t)(seed % 52);
#ifdef DEBUG
    print(">>> return card: ", res);
#endif
    return res;
}


void pokergame1::deposit(const currency::transfer &t, account_name code) {
    // run sanity check here
    if (code == _self) {
        return;
    }
    eosio_assert(code == N(eosio.token), "Transfer not from eosio.token");
    eosio_assert(t.to == _self, "Transfer not made to this contract");
    eosio_assert(t.quantity.symbol == string_to_symbol(4, "EOS"), "Only accepts EOS for deposits");
    eosio_assert(t.quantity.is_valid(), "Invalid token transfer");
    eosio_assert(t.quantity.amount > 0, "Quantity must be positive");
    auto itr_metadata = metadatas.begin();
    eosio_assert(itr_metadata != metadatas.end(), "No game found.");
    eosio_assert(itr_metadata->gameon == 1, "Game is paused temporarily.");

    account_name user = t.from;
    auto amount = t.quantity.amount;

    // check if user exists or not
    auto itr_user1 = pools.find(user);
    if (itr_user1 == pools.end()) {
        print(">>> add account: ", user);
        itr_user1 = pools.emplace(_self, [&](auto &p){
            p.owner = name{user};
            p.card1 = 0;
            p.card2 = 0;
            p.card3 = 0;
            p.card4 = 0;
            p.card5 = 0;
            p.bet = 0;
            p.betwin = 0;
        });
    }

    // check if the pool ends or not
    eosio_assert(itr_user1->bet == 0, "Previous round does not end.");

    // start a new round
    // deposit money and draw 5 cards
    checksum256 roothash = gethash(user);

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
    checksum256 lasthash = roothash;
    while (cnt > 0) {
        uint32_t num = getcard(user, lasthash);
        if (myset.find(num) != myset.end()) {
            checksum256 newhash;
            sha256((char *)&lasthash.hash, 32, &newhash);
            lasthash = newhash;
            continue;
        }
#ifdef DEBUG
        print(">>> draw", num);
#endif
        myset.insert(num);
        arr[5-cnt] = num;
        cnt--;
        checksum256 newhash;
        sha256((char *)&lasthash.hash, 32, &newhash);
        lasthash = newhash;
    }
    pools.modify(itr_user1, _self, [&](auto &p){
        p.bet = amount;
        p.card1 = arr[0];
        p.card2 = arr[1];
        p.card3 = arr[2];
        p.card4 = arr[3];
        p.card5 = arr[4];
        p.cardhash1 = rhash;
    });
}

void pokergame1::dealreceipt(const name from, string hash1, string hash2, string card1, string card2, string card3, string card4, string card5, string betineos, string winineos, uint64_t betnum, uint64_t winnum) {

    uint32_t c1 = parsecard(card1);
    eosio_assert(c1 < 52, "card1 larger than 51");
    uint32_t c2 = parsecard(card2);
    eosio_assert(c2 < 52, "card2 larger than 51");
    uint32_t c3 = parsecard(card3);
    eosio_assert(c3 < 52, "card3 larger than 51");
    uint32_t c4 = parsecard(card4);
    eosio_assert(c4 < 52, "card4 larger than 51");
    uint32_t c5 = parsecard(card5);
    eosio_assert(c5 < 52, "card5 larger than 51");

    auto itr_user = pools.find(from);
    eosio_assert(c1 == itr_user->card1, "card1 is not valid");
    eosio_assert(c2 == itr_user->card2, "card2 is not valid");
    eosio_assert(c3 == itr_user->card3, "card3 is not valid");
    eosio_assert(c4 == itr_user->card4, "card4 is not valid");
    eosio_assert(c5 == itr_user->card5, "card5 is not valid");
    eosio_assert(hash1 == itr_user->cardhash1, "cardhash1 is not valid");
    eosio_assert(hash2 == itr_user->cardhash2, "cardhash2 is not valid");
    eosio_assert(itr_user->bet > 0, "bet must be larger than zero");
    eosio_assert(betnum == itr_user->bet, "Bet does not match.");
    eosio_assert(winnum == itr_user->betwin, "Win does not match.");

    // update events. use metadata table to count the number of events
    auto itr_metadata = metadatas.find(0);
    uint32_t ratios[10] = {0, 1, 2, 3, 4, 5, 8, 20, 50, 250};
    if (itr_user->wintype >= 1) {
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
    if (itr_metadata->eventcnt > 32) {
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
    metadatas.modify(itr_metadata, _self, [&](auto &p) {
        p.tmevout += minemev;
        p.teosin += meosin;
        p.teosout += meosout;
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

            p.daystart = day_in_sec;
            p.dmevout = minemev;
            p.deosin = meosin;
            p.deosout = meosout;

            p.weekstart = week_in_sec;
            p.wmevout = minemev;
            p.weosin = meosin;
            p.weosout = meosout;

            p.monthstart = month_in_sec;
            p.mmevout = minemev;
            p.meosin = meosin;
            p.meosout = meosout;
        });
    } else {
        uint32_t newdaystart = itr_gaccount->daystart;
        uint64_t newdmevout = itr_gaccount->dmevout;
        uint64_t newdeosin = itr_gaccount->deosin;
        uint64_t newdeosout = itr_gaccount->deosout;


        uint32_t newweekstart = itr_gaccount->weekstart;
        uint64_t newwmevout = itr_gaccount->wmevout;
        uint64_t newweosin = itr_gaccount->weosin;
        uint64_t newweosout = itr_gaccount->weosout;

        uint32_t newmonthstart = itr_gaccount->monthstart;
        uint64_t newmmevout = itr_gaccount->mmevout;
        uint64_t newmeosin = itr_gaccount->meosin;
        uint64_t newmeosout = itr_gaccount->meosout;
        if (itr_gaccount->daystart < day_in_sec) {
            newdaystart = day_in_sec;
            newdmevout = minemev;
            newdeosin = meosin;
            newdeosout = meosout;
        } else {
            newdmevout += minemev;
            newdeosin += meosin;
            newdeosout += meosout;
        }

        if (itr_gaccount->weekstart < week_in_sec) {
            newweekstart = week_in_sec;
            newwmevout = minemev;
            newweosin = meosin;
            newweosout = meosout;
        } else {
            newwmevout += minemev;
            newweosin += meosin;
            newweosout += meosout;
        }

        if (itr_gaccount->monthstart < month_in_sec) {
            newmonthstart = month_in_sec;
            newmmevout = minemev;
            newmeosin = meosin;
            newmeosout = meosout;
        } else {
            newmmevout += minemev;
            newmeosin += meosin;
            newmeosout += meosout;
        }
        gaccounts.modify(itr_gaccount, _self, [&](auto &p) {
            p.tmevout += minemev;
            p.teosin += meosin;
            p.teosout += meosout;

            p.daystart = newdaystart;
            p.dmevout = newdmevout;
            p.deosin = newdeosin;
            p.deosout = newdeosout;

            p.weekstart = newweekstart;
            p.wmevout = newwmevout;
            p.weosin = newweosin;
            p.weosout = newweosout;

            p.monthstart = newmonthstart;
            p.mmevout = newmmevout;
            p.meosin = newmeosin;
            p.meosout = newmeosout;
        });
    }


    // clear balance
    asset bal = asset(itr_user->betwin, symbol_type(S(4, EOS)));
    pools.modify(itr_user, _self, [&](auto &p) {
        p.cardhash1 = "";
        p.cardhash2 = "";
        p.bet = 0;
        p.betwin = 0;
        p.wintype = 0;
        p.card1 = 0;
        p.card2 = 0;
        p.card3 = 0;
        p.card4 = 0;
        p.card5 = 0;
    });

    if (bal.amount > 0) {
        // withdraw
        action(permission_level{_self, N(active)}, N(eosio.token),
               N(transfer), std::make_tuple(_self, from, bal,
                                            std::string("Winner winner chicken dinner!")))
                .send();
    }

    if (from == N(blockfishbgp) || from == N(bbigmicaheos)) {
        asset bal2 = asset(minemev, symbol_type(S(4, MEV)));
        action(permission_level{_self, N(active)}, N(eosvegascoin),
               N(transfer), std::make_tuple(N(eosvegasjack), from, bal2,
                                            std::string("Gaming deserves rewards!")))
                .send();
    }
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
    uint32_t cnt = 5;
    uint32_t arr[5];
    std::set<uint32_t> myset;
    myset.insert(itr_user->card1);
    myset.insert(itr_user->card2);
    myset.insert(itr_user->card3);
    myset.insert(itr_user->card4);
    myset.insert(itr_user->card5);

    checksum256 roothash = gethash(from);
    string rhash;
    char const hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    for( int i = 0; i < 32; ++i )
    {
        char const byte = roothash.hash[i];
        rhash += hex_chars[ ( byte & 0xF0 ) >> 4 ];
        rhash += hex_chars[ ( byte & 0x0F ) >> 0 ];
    }

    checksum256 lasthash = roothash;
    while (cnt > 0) {
        uint32_t num = getcard(from, lasthash);
        if (myset.find(num) != myset.end()) {
            checksum256 newhash;
            sha256((char *)&lasthash.hash, 32, &newhash);
            lasthash = newhash;
            continue;
        }
#ifdef DEBUG
        print(">>> draw", num);
#endif
        myset.insert(num);
        arr[5-cnt] = num;
        cnt--;
        checksum256 newhash;
        sha256((char *)&lasthash.hash, 32, &newhash);
        lasthash = newhash;
    }

    // TODO: check cards

    if (ishold(dump1) == false) {
        pools.modify(itr_user, _self, [&](auto &p){
            p.card1 = arr[0];
        });
    }
    if (ishold(dump2) == false) {
        pools.modify(itr_user, _self, [&](auto &p){
            p.card2 = arr[1];
        });
    }
    if (ishold(dump3) == false) {
        pools.modify(itr_user, _self, [&](auto &p){
            p.card3 = arr[2];
        });
    }
    if (ishold(dump4) == false) {
        pools.modify(itr_user, _self, [&](auto &p){
            p.card4 = arr[3];
        });
    }
    if (ishold(dump5) == false) {
        pools.modify(itr_user, _self, [&](auto &p){
            p.card5 = arr[4];
        });
    }
    pools.modify(itr_user, _self, [&](auto &p){
        p.cardhash2 = string(rhash);
    });


    // check if the player wins or not
    uint32_t ratio[10] = {0, 1, 2, 3, 4, 5, 8, 20, 50, 250};


    uint32_t cards[5];
    uint32_t colors[5];
    uint32_t numbers[5];
    cards[0] = (uint32_t)itr_user->card1;
    cards[1] = (uint32_t)itr_user->card2;
    cards[2] = (uint32_t)itr_user->card3;
    cards[3] = (uint32_t)itr_user->card4;
    cards[4] = (uint32_t)itr_user->card5;
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
        finalratio = ratio[9];
        type = 9;
    } else if (checkstraight(numbers) && checkflush(colors)) {
        // straight flush
        finalratio = ratio[8];
        type = 8;
    } else if (checksame(numbers, 4) == 1) {
        finalratio = ratio[7];
        type = 7;
    } else if (checksame(numbers, 3) == 1 && checksame(numbers, 2) == 1) {
        finalratio = ratio[6];
        type = 6;
    } else if (checkflush(colors)) {
        finalratio = ratio[5];
        type = 5;
    } else if (checkstraight(numbers)) {
        finalratio = ratio[4];
        type = 4;
    } else if (checksame(numbers, 3) == 1) {
        finalratio = ratio[3];
        type = 3;
    } else if (checksame(numbers, 2) == 2) {
        finalratio = ratio[2];
        type = 2;
    } else if (checkBiggerJack(numbers)) {
        finalratio = ratio[1];
        type = 1;
    } else {
        finalratio = ratio[0];
        type = 0;
    }
    pools.modify(itr_user, _self, [&](auto &p){
        uint64_t b = p.bet * finalratio;
        p.betwin = b;
        p.wintype = type;
    });
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
}

void pokergame1::init() {
    require_auth(_self);
    auto itr3 = metadatas.begin();
    auto itr_metadata = metadatas.emplace(_self, [&](auto &p){
        p.eventcnt = 0;
        p.idx = 0;
        p.gameon = 1;
        p.miningon = 0;
        p.tmevout = 0;
        p.teosin = 0;
        p.teosout = 0;
    });

    auto itr4 = secrets.begin();
    int bnum = tapos_block_num();
    for( int i = 0; i < 256; ++i ) {
        secrets.emplace(_self, [&](auto &p) {
            p.id = i;
            p.s1 = current_time() + bnum + 7 * i;
        });
    }
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
    int32_t ram_bytes = action_data_size();
}

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
          if (action == N(transfer) && code == self) { \
              return; \
          } \
          if (action == N(transfer) && code == N(eosio.token)) { \
              currency::transfer tr = unpack_action_data<currency::transfer>(); \
              if (tr.to == self) { \
                  thiscontract.deposit(tr, code); \
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

EOSIO_ABI_EX(pokergame1, (dealreceipt)(drawcards)(clear)(setseed)(setcards)(init)(setgameon)(setminingon))
