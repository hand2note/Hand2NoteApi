#ifndef _H2NAPIDLL__
#define _H2NAPIDLL__

#ifdef h2napidll_EXPORTS
	#define H2N_API __declspec(dllexport)
#else
	#ifdef H2NAPI_SRC_UNIT_TESTS
		#define H2N_API 
	#else
		#define H2N_API __declspec(dllimport)
	#endif
#endif


#ifdef __cplusplus
extern "C" {
#endif

#define H2N_HHFMT_ORIGINAL 0
#define H2N_HHFMT_STARS 1
#define H2N_HHFMT_PACIFIC 2
#define H2N_HHFMT_WPN 3

#define H2N_ROOM_UNRECOGNIZED 0
#define H2N_ROOM_POKERSTARS 10
#define H2N_ROOM_PACIFIC 14
#define H2N_ROOM_IPOKER 13
#define H2N_ROOM_PARTYPOKER 11
#define H2N_ROOM_WINNINGPOKERNETWORK 15
#define H2N_ROOM_WINAMAX 9
#define H2N_ROOM_MICROGAMING 7
#define H2N_ROOM_BAAZI 1
#define H2N_ROOM_CHECKRAISE 2
#define H2N_ROOM_DOLLARO 12
#define H2N_ROOM_BETSENSE 3
#define H2N_ROOM_MONKEYBET 4
#define H2N_ROOM_FULPOT 5
#define H2N_ROOM_GGNET 16
#define H2N_ROOM_ENETPOKER 17
#define H2N_ROOM_KLASPOKER 18
#define H2N_ROOM_POKERWORLD 19
#define H2N_ROOM_OURGAME 20
#define H2N_ROOM_BETONLINE 21
#define H2N_ROOM_BIGBETGE 22
#define H2N_ROOM_BLUFFONLINE 23
#define H2N_ROOM_BLUFFDADDY 24
#define H2N_ROOM_COLOMBIAPOKERLIVE 25
#define H2N_ROOM_EUROPEBETCOM 26
#define H2N_ROOM_FTRPOKER 27
#define H2N_ROOM_POKERGDFPLAY 28
#define H2N_ROOM_HIGHROLLERS 29
#define H2N_ROOM_ITALYLIVEPOKER 30
#define H2N_ROOM_POKERMANIA 31
#define H2N_ROOM_POKERMIRA 32
#define H2N_ROOM_POKERDOM 33
#define H2N_ROOM_POKERMATCH 34
#define H2N_ROOM_REDARGENTINADEPOKER 35
#define H2N_ROOM_SEKABETCOM 36
#define H2N_ROOM_SEKABET 37
#define H2N_ROOM_SPARTANPOKERCOM 38
#define H2N_ROOM_SPORTSBETTING 39
#define H2N_ROOM_TIGERGAMING 40
#define H2N_ROOM_VENEZUELAPOKERLIVE 41
#define H2N_ROOM_XMASTER 42
#define H2N_ROOM_POKERGRANT 43
#define H2N_ROOM_GRANDPOKEREU 44
#define H2N_ROOM_REVOLUTIONBETS 45
#define H2N_ROOM_VBET 46
#define H2N_ROOM_WIN2DAY 47
#define H2N_ROOM_WWIN 48
#define H2N_ROOM_POKERMASTER 49
#define H2N_ROOM_PLANETWIN365 50
#define H2N_ROOM_ACONCAGUAPOKER 51
#define H2N_ROOM_BRASILPOKERLIVE 52
#define H2N_ROOM_SURPOKERDELASAMERICAS 53
#define H2N_ROOM_CHILEPOKERLIVE 54
#define H2N_ROOM_BOLIVIAPOKERLIVE 55
#define H2N_ROOM_COSTARICAPOKERLIVE 56
#define H2N_ROOM_GUARANIPOKERLIVE 57
#define H2N_ROOM_MEXICOPOKERLIVE 58
#define H2N_ROOM_PERUPOKERLIVE 59
#define H2N_ROOM_PPPOKER 60
#define H2N_ROOM_POKERKINGDOM 61
#define H2N_ROOM_POKERKING 62
#define H2N_ROOM_FISHPOKERS 63
#define H2N_ROOM_OHPOKER 64
#define H2N_ROOM_ONEPS 65
#define H2N_ROOM_POKERCLANS 66
#define H2N_ROOM_KKPOKER 67

#define H2N_CURRENCY_DOLLAR 1
#define H2N_CURRENCY_EURO 2
#define H2N_CURRENCY_POUND 3
#define H2N_CURRENCY_PLAYMONEY 4
#define H2N_CURRENCY_POINTS 5
#define H2N_CURRENCY_CHIPS 6
#define H2N_CURRENCY_YUAN 7
#define H2N_CURRENCY_INDIANRUPEE 8
#define H2N_CURRENCY_HRYVNIA 9
#define H2N_CURRENCY_ROUBLE 10
#define H2N_CURRENCY_GEORGIANLARI 11
#define H2N_CURRENCY_UNDEFINED 0

#define H2N_ACTION_FOLD 0
#define H2N_ACTION_CALL 1
#define H2N_ACTION_RAISE 2
#define H2N_ACTION_CHECK 3
#define H2N_ACTION_BET 4

#define H2N_STREET_PREFLOP 0
#define H2N_STREET_FLOP 3
#define H2N_STREET_TURN 4
#define H2N_STREET_RIVER 5

#define H2N_COMMAND_CLOSEHUD 0
#define H2N_COMMAND_REOPENTABLE 3
#define H2N_COMMAND_RESTARTEMULATOR 4


#define H2N_MAX_SEATS 10

typedef struct {
	int         room;
	int         is_zoom;
	double      gameid;
	int         format;
	const char* hh_formatted;
	const char* hh_original;
} h2n_hh_message;

typedef struct {
	int         seat_idx;
	const char* nickname;
	const char* player_id;
	double      stack;
	const char* pocket_cards;
	int         is_dealer;
	int         is_posted_sb;
	int         is_posted_bb;
	int         is_posted_sb_outofqueue;
	int         is_posted_bb_outofqueue;
	int         is_posted_straddle;
	int         is_hero;
	int         is_sitting_out;
} h2n_seat_info;

typedef struct {
	int         room;
	double      gameid;

	const char* table_name;
	int         table_hwnd;

	int         max_players;

	int         is_tourney;
	int         is_omaha;
	int         is_limit;
	int         is_zoom;
	int         is_cap;
	int         is_potlimit;
	int         is_shortdeck;
	int         is_omahafive;

	int         currency;

	double      sb;
	double      bb;
	double      ante;
	double      straddle;

	h2n_seat_info seats[H2N_MAX_SEATS];
	int           seats_num;
} h2n_start_hand_message;

typedef struct {
	double      gameid;
	int         seat_idx;
	int         type;
	double      amount;
	int         is_allin;
	double      pot;
} h2n_action_message;

typedef struct {
	double      gameid;
	int         type;
	const char* board;
	double      pot;
} h2n_street_message;

H2N_API int h2n_is_running();

H2N_API char* h2n_make_table_name(int room, const char* original_name);
H2N_API void h2n_free_cstring(char* str);

H2N_API int h2n_send_handhistory(h2n_hh_message* msg);
H2N_API int h2n_send_hand_start(h2n_start_hand_message* msg);
H2N_API int h2n_send_action(h2n_action_message* msg);
H2N_API int h2n_send_street(h2n_street_message* msg);
H2N_API int h2n_send_json(const char* json_str);
H2N_API int h2n_send_command(int table_hwnd, int room_id, int cmd);




#ifdef __cplusplus
}
#endif

#endif
