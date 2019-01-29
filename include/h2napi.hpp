#ifndef _H2NAPIHPP__
#define _H2NAPIHPP__

#include "h2napi.h"
#include <thread>
#include <atomic>
#include <string>
#include <vector>
namespace Hand2Note {

	class ClientStatusChecker {
	public:
		virtual ~ClientStatusChecker() {}

		bool IsRunning() const {
			return h2n_is_running() != 0;
		}
	};

	class ClientControl : public ClientStatusChecker {
	public:

		virtual void OnStart() = 0;
		virtual void OnClose() = 0;

		ClientControl() :loop_(true) {
			poll_thread_ = std::thread([this]() { DoWork(); });
		}
		
		virtual ~ClientControl() {
			loop_ = false;
			poll_thread_.join();
		}

	private:

		void DoWork() {
			using namespace std::literals;
			is_running_ = IsRunning();
			if (is_running_)
				OnStart();

			while (loop_) {
				std::this_thread::sleep_for(300ms);
				bool curr = IsRunning();
				if (curr == is_running_)
					continue;
				is_running_ = curr;
				if (is_running_)
					OnStart();
				else
					OnClose();
			}
		}

		bool is_running_;
		std::atomic_bool loop_;
		std::thread poll_thread_;

	};


	enum class HandHistoryFormat : int
	{
		PokerStars = H2N_HHFMT_POKERSTARS,
		Pacific = H2N_HHFMT_888,
		WPN = H2N_HHFMT_WPN,
	};

	enum class Room : int
	{
		PokerStars = H2N_ROOM_POKERSTARS,
		Pacific = H2N_ROOM_888,
		PokerMaster = H2N_ROOM_POKERMASTER,
		PokerFish = H2N_ROOM_POKERFISH,
	};

	enum class Currency : int
	{
		Dollar = H2N_CURRENCY_USD,
		Euro = H2N_CURRENCY_EUR,
		Pounds = H2N_CURRENCY_GBP,
		Chips = H2N_CURRENCY_CHP,
		Yuan = H2N_CURRENCY_YUAN,
		IndianRupee = H2N_CURRENCY_INR,
		Hryvnia = H2N_CURRENCY_UAH,
		Rouble = H2N_CURRENCY_RUB,
		GeorgianLari = H2N_CURRENCY_GEL,
		BrazilianReal = H2N_CURRENCY_BRL,
	};

	class Utils {
	public:
		inline static std::string MakeTableName(Room room, const std::string& OriginalTableName) {
			char *tn = h2n_make_table_name((int)room, OriginalTableName.c_str());
			std::string ret(tn);
			h2n_free_cstring(tn);
			return ret;
		}
	};

	class HandHistoryMessage {
	public:
		HandHistoryMessage() :
			room_(Room::PokerStars), format_(HandHistoryFormat::PokerStars), game_id_(0), is_zoom_(false)
		{
		}

		HandHistoryMessage(Room room, uint64_t game_id, HandHistoryFormat format, const std::string& FormattedHandHistory) :
			room_(room), format_(format), game_id_(game_id), fhh_(FormattedHandHistory), is_zoom_(false)
		{
		}

		const std::string& FormattedHandHistory() const { return fhh_; }
		void FormattedHandHistory(const std::string& FormattedHandHistory_) { fhh_ = FormattedHandHistory_; }

		Room room() const { return room_; }
		void room(Room r) { room_ = r; }

		bool IsZoom() const { return is_zoom_; }
		void SetZoom(bool z) { is_zoom_ = z; }

		HandHistoryFormat Format() const { return format_; }
		void Format(HandHistoryFormat f) { format_ = f; }

		uint64_t GameId() const { return game_id_; }
		void GameId(uint64_t id) { game_id_ = id; }

		const std::string& OriginalHandHistory() const { return ohh_; }
		void OriginalHandHistory(const std::string& hh) { ohh_ = hh; }
	private:
		std::string fhh_;
		std::string ohh_;
		HandHistoryFormat format_;
		Room room_;
		uint64 game_id_;
		bool is_zoom_;

		void MakeH2NApiLibMessage(h2n_hh_message* msg) const {
			msg->format = (int)format_;
			msg->gameid = (double) game_id_;
			msg->is_zoom = is_zoom_ ? 1 : 0;
			msg->room = (int)room_;
			msg->hh_formatted = fhh_.c_str();
			msg->hh_original = ohh_.c_str();
		}

		friend class Protocol;
	};


	class SeatInfo {
	public:
		SeatInfo() :
			seat_idx_(0), stack_(0), is_hero_(false), player_id_(0), is_dealer_(false), 
			is_posted_bb_(false), is_posted_sb_(false), is_posted_bb_outofqueue_(false), 
			is_posted_sb_outofqueue_(false), is_posted_straddle_(false), is_sitting_out_(false)
		{
		}

		SeatInfo(const std::string& name, int index, double stack, bool is_hero = false):
			nickname_(name), seat_idx_(index), stack_(stack), is_hero_(is_hero),
			player_id_(0), is_dealer_(false), is_posted_bb_(false), is_posted_sb_(false),
			is_posted_bb_outofqueue_(false), is_posted_sb_outofqueue_(false), is_posted_straddle_(false),
			is_sitting_out_(false)
		{
		}

		SeatInfo(uint64 player_id, int index, double stack, bool is_hero = false): 
			seat_idx_(index), stack_(stack), is_hero_(is_hero),
			player_id_(player_id), is_dealer_(false), is_posted_bb_(false), is_posted_sb_(false),
			is_posted_bb_outofqueue_(false), is_posted_sb_outofqueue_(false), is_posted_straddle_(false),
			is_sitting_out_(false)
		{
		}

		int SeatIndex() const { return seat_idx_; }
		void SeatIndex(int idx) { seat_idx_ = idx; }

		const std::string& Nickname() const { return nickname_; }
		void Nickname(const std::string& nick) { nickname_ = nick; }

		uint64 PlayerId() const { return player_id_; }
		void PlayerId(uint64 pid) { player_id_ = pid; }

		double Stack() const { return stack_; }
		void Stack(double s) { stack_ = s; }

		const std::string& PoketCards() const { return pocket_cards_; }
		void PoketCards(const std::string& pocket_cards) { pocket_cards_ = pocket_cards; }

		bool IsDealer() const { return is_dealer_; }
		void SetDealer(bool is_dealer) { is_dealer_ = is_dealer; }

		bool IsHero() const { return is_hero_; }
		void SetHero(bool hero) { is_hero_ = hero; }

		bool IsPostedSmallBlind() const { return is_posted_sb_; }
		void SetPostedSmallBlind(bool posted) { is_posted_sb_ = posted; }

		bool IsPostedBigBlind() const { return is_posted_bb_; }
		void SetPostedBigBlind(bool posted) { is_posted_bb_ = posted; }

		bool IsPostedBigBlindOutOfQueue() const { return is_posted_bb_outofqueue_; }
		void SetPostedBigBlindOutOfQueue(bool posted) { is_posted_bb_outofqueue_ = posted; }

		bool IsPostedSmallBlindOutOfQueue() const { return is_posted_sb_outofqueue_; }
		void SetPostedSmallBlindOutOfQueue(bool posted) { is_posted_sb_outofqueue_ = posted; }

		bool IsPostedStaddle() const { return is_posted_straddle_; }
		void SetPostedStraddle(bool posted) { is_posted_straddle_ = posted; }

		bool IsSittingOut() const { return is_sitting_out_; }
		void SetSittingOut(bool sitout) { is_sitting_out_ = sitout; }

	private:
		int          seat_idx_;
		std::string  nickname_;
		uint64       player_id_;
		double       stack_;
		std::string  pocket_cards_;
		bool         is_dealer_;
		bool         is_posted_sb_;
		bool         is_posted_bb_;
		bool         is_posted_sb_outofqueue_;
		bool         is_posted_bb_outofqueue_;
		bool         is_posted_straddle_;
		bool         is_hero_;
		bool         is_sitting_out_;

		void MakeH2NApiSeatInfo(h2n_seat_info* s) const {
			s->is_dealer = is_dealer_ ? 1 : 0;
			s->is_hero = is_hero_ ? 1 : 0;
			s->is_posted_bb = is_posted_bb_ ? 1 : 0;
			s->is_posted_bb_outofqueue = is_posted_bb_outofqueue_ ? 1 : 0;
			s->is_posted_sb = is_posted_sb_ ? 1 : 0;
			s->is_posted_sb_outofqueue = is_posted_sb_outofqueue_ ? 1 : 0;
			s->is_posted_straddle = is_posted_straddle_ ? 1 : 0;
			s->is_sitting_out = is_sitting_out_ ? 1 : 0;
			s->nickname = nickname_.c_str();
			s->player_id = (double)player_id_;
			s->pocket_cards = pocket_cards_.c_str();
			s->seat_idx = seat_idx_;
			s->stack = stack_;
		}

		friend class HandStartMessage;

	};

	class HandStartMessage {
	public:
		typedef std::vector<SeatInfo> SeatsList;

		HandStartMessage() :
			room_(Room::PokerStars), game_id_(0), table_hwnd_(0), max_players_(0),
			is_tourney_(false), is_omaha_(false), is_limit_(false), is_zoom_(false),
			is_cap_(false), is_potlimit_(false), currency_(Currency::Dollar), sb_(0), bb_(0),
			ante_(0), straddle_(0)
		{}
		HandStartMessage(Room room, uint64 gameid = 0, int table_hwnd = 0) :
			room_(room), game_id_(gameid), table_hwnd_(table_hwnd), max_players_(0),
			is_tourney_(false), is_omaha_(false), is_limit_(false), is_zoom_(false),
			is_cap_(false), is_potlimit_(false), currency_(Currency::Dollar), sb_(0), bb_(0),
			ante_(0), straddle_(0)
		{}

		Room room() const {	return room_; }
		void room(Room r) {	room_ = r; }

		uint64_t GameId() const { return game_id_; }
		void GameId(uint64_t id) { game_id_ = id; }

		const std::string& TableName() const { return table_name_; }
		void TableName(const std::string& name) { table_name_ = name; }

		int TableHwnd() const { return table_hwnd_; }
		void TableHwnd(int hwnd) { table_hwnd_ = hwnd; }

		int MaxPlayers() const { return max_players_; }
		void MaxPlayers(int max_players) { max_players_ = max_players; }
		
		bool IsTourney() const { return is_tourney_; }
		void SetTourney(bool tourney) { is_tourney_ = tourney; }

		bool IsOmaha() const { return is_omaha_; }
		void SetOmaha(bool omaha) { is_omaha_ = omaha; }

		bool IsLimit() const { return is_limit_; }
		void SetLimit(bool lim) { is_limit_ = lim; }

		bool IsZoom() const { return is_zoom_; }
		void SetZoom(bool z) { is_zoom_ = z; }

		bool IsCap() const { return is_cap_; }
		void SetCap(bool cap) { is_cap_ = cap; }

		bool IsPotLimit() const { return is_potlimit_; }
		void SetPotLimit(bool potlim) { is_potlimit_ = potlim; }

		Currency currency() const { return currency_; }
		void SetCurrency(Currency curr) { currency_ = curr; }

		double SmallBlind() const { return sb_; }
		void SmallBlind(double sb) { sb_ = sb; }

		double BigBlind() const { return bb_; }
		void BigBlind(double bb) { bb_ = bb; }

		double Ante() const { return ante_; }
		void Ante(double ante) { ante_ = ante; }

		double Straddle() const { return straddle_; }
		void Straddle(double straddle) { straddle_ = straddle; }

		void Seats(const SeatsList& seats) { seats_ = seats; }
		const SeatsList& Seats() const { return seats_; }
	private:
		Room        room_;
		uint64      game_id_;
		std::string table_name_;
		int         table_hwnd_;
		int         max_players_;
		bool        is_tourney_;
		bool        is_omaha_;
		bool        is_limit_;
		bool        is_zoom_;
		bool        is_cap_;
		bool        is_potlimit_;
		Currency    currency_;
		double      sb_;
		double      bb_;
		double      ante_;
		double      straddle_;
		SeatsList   seats_;

		friend class Protocol;

		void MakeH2NApiLibMessage(h2n_start_hand_message* msg) const {
			msg->ante = ante_;
			msg->bb = bb_;
			msg->currency = (int)currency_;
			msg->gameid = (double)game_id_;
			msg->is_cap = is_cap_ ? 1 : 0;
			msg->is_limit = is_limit_ ? 1 : 0;
			msg->is_omaha = is_omaha_ ? 1 : 0;
			msg->is_potlimit = is_potlimit_ ? 1 : 0;
			msg->is_tourney = is_tourney_ ? 1 : 0;
			msg->is_zoom = is_zoom_ ? 1 : 0;
			msg->max_players = max_players_;
			msg->room = (int)room_;
			msg->sb = sb_;
			msg->straddle = straddle_;
			msg->table_hwnd = table_hwnd_;
			msg->table_name = table_name_.c_str();

			msg->seats_num = 0;
			for (SeatsList::const_iterator it = seats_.cbegin(); it != seats_.cend(); ++it) {
				it->MakeH2NApiSeatInfo( &(msg->seats[msg->seats_num++]));
			}
		}
	};

	enum class Action : int
	{
		Fold = H2N_ACTION_FOLD,
		Call = H2N_ACTION_CALL,
		Raise = H2N_ACTION_RAISE,
		Bet = H2N_ACTION_BET,
		Check = H2N_ACTION_CHECK,
	};

	enum class Street : int
	{
		Flop = H2N_STREET_FLOP,
		Turn = H2N_STREET_TURN,
		River = H2N_STREET_RIVER,
	};

	class HandActionMessage {
	public:
		HandActionMessage() :
			game_id_(0), seat_idx_(0), type_(Action::Fold), amount_(0), is_allin_(false), pot_(0)
		{
		}

		HandActionMessage(uint64_t game_id, int seat_id, Action type, double amount, bool is_allin = false) :
			game_id_(game_id), seat_idx_(seat_id), type_(type), amount_(amount), is_allin_(is_allin), pot_(0)
		{
		}

		uint64_t GameId() const { return game_id_; }
		void GameId(uint64_t id) { game_id_ = id; }

		int SeatIndex() const { return seat_idx_; }
		void SeatIndex(int idx) { seat_idx_ = idx; }

		Action ActionType() const { return type_; }
		void ActionType(Action type) { type_ = type; }

		double Amount() const { return amount_; }
		void Amount(double val) { amount_ = val; }

		bool IsAllin() const { return is_allin_; }
		void SetAllin(bool allin) { is_allin_ = allin; }

		double Pot() const { return pot_; }
		void Pot(double val) { pot_ = val; }

	private:
		uint64      game_id_;
		int         seat_idx_;
		Action      type_;
		double      amount_;
		bool        is_allin_;
		double      pot_;

		void MakeH2NApiLibMessage(h2n_action_message* msg) const {
			msg->amount = amount_;
			msg->gameid = (double)game_id_;
			msg->is_allin = is_allin_ ? 1 : 0;
			msg->pot = pot_;
			msg->seat_idx = seat_idx_;
			msg->type = (int)type_;
		}

		friend class Protocol;
	};

	class HandStreetMessage {
	public:
		HandStreetMessage() :
			game_id_(0), type_(Street::Flop), pot_(0)
		{
		}

		HandStreetMessage(uint64_t game_id, Street type, const std::string& board, double pot = 0) :
			game_id_(game_id), type_(type), board_(board), pot_(0)
		{
		}

		uint64_t GameId() const { return game_id_; }
		void GameId(uint64_t id) { game_id_ = id; }

		const std::string& Board() const { return board_; }
		void Board(const std::string& board) { board_ = board; }

		Street StreetType() const { return type_; }
		void StreetType(Street type) { type_ = type; }

		double Pot() const { return pot_; }
		void Pot(double val) { pot_ = val; }

	private:
		uint64      game_id_;
		Street      type_;
		str         board_;
		double      pot_;

		void MakeH2NApiLibMessage(h2n_street_message* msg) const {
			msg->gameid = (double)game_id_;
			msg->board = board_.c_str();
			msg->type = (int)type_;
			msg->pot = pot_;
		}

		friend class Protocol;
	};


	class Protocol {
	public:
		inline static int SendHandHistory(const HandHistoryMessage& msg) {
			h2n_hh_message m;
			msg.MakeH2NApiLibMessage(&m);
			return h2n_send_handhistory(&m);
		}
		inline static int SendHandStart(const HandStartMessage& msg) {
			h2n_start_hand_message m;
			msg.MakeH2NApiLibMessage(&m);
			return h2n_send_hand_start(&m);
		}
		inline static int SendHandActon(const HandActionMessage& msg) {
			h2n_action_message m;
			msg.MakeH2NApiLibMessage(&m);
			return h2n_send_action(&m);
		}
		inline static int SendHandStreed(const HandStreetMessage& msg) {
			h2n_street_message m;
			msg.MakeH2NApiLibMessage(&m);
			return h2n_send_street(&m);
		}

	};
}


#endif
