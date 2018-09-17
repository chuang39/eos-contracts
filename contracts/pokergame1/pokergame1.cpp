#include "pokergame1.hpp"

#define DEBUG 1
// What's this table used for? God knows!

checksum256 pokergame1::gethash(account_name from) {
    checksum256 result;
    int bnum = tapos_block_num();
    uint64_t seed = current_time() + from + bnum;
    sha256((char *)&seed, sizeof(seed), &result);
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

void pokergame1::dealreceipt(const name from, string hash1, string hash2, string card1, string card2, string card3, string card4, string card5, uint64_t bet, uint64_t win) {

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
    eosio_assert(c2 == itr_user->card2, "card1 is not valid");
    eosio_assert(c3 == itr_user->card3, "card1 is not valid");
    eosio_assert(c4 == itr_user->card4, "card1 is not valid");
    eosio_assert(c5 == itr_user->card5, "card1 is not valid");
    eosio_assert(hash1 == itr_user->cardhash1, "cardhash1 is not valid");
    eosio_assert(hash2 == itr_user->cardhash2, "cardhash2 is not valid");
    eosio_assert(itr_user->bet > 0, "bet must be larger than zero");
    eosio_assert(bet == itr_user->bet, "Bet does not match.");
    eosio_assert(win == itr_user->betwin, "Win does not match.");


    // update events. use metadata table to count the number of events
    auto itr_metadata = metadatas.begin();
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
                                            std::string("See you at MyEosVegas.com next time!")))
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

    auto itr_metadata = metadatas.emplace(_self, [&](auto &p){
        p.eventcnt = 0;
    });
}

void pokergame1::setseed(const name from, uint32_t seed) {}

void pokergame1::setcards(const name from, uint32_t c1, uint32_t c2, uint32_t c3, uint32_t c4, uint32_t c5) {
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
      if(self == N(eosvegasjack) && (code == self || code == N(eosio.token) || action == N(onerror))) { \
         print("=======", name{self}); \
         print("=======", name{code}); \
         TYPE thiscontract(self); \
         if (action == N(transfer)) { \
             currency::transfer tr = unpack_action_data<currency::transfer>(); \
             if (tr.to == self) { \
                 thiscontract.deposit(tr, code); \
             } \
             return; \
         } \
         if (action == N(pong2)) { \
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

EOSIO_ABI_EX(pokergame1, (dealreceipt)(drawcards)(clear)(setseed)(setcards))
