#ifndef _H2NAPIHPP__
#define _H2NAPIHPP__

#include "h2napi.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <string>
#include <vector>
#include <condition_variable>

namespace Hand2Note {

	/// <summary>
	/// Hand history text formats supported by Hand2Note. Used to send completed hands.
	/// </summary>
	enum class HandHistoryFormats : int
	{
		Original = 0,
		Stars = 1,
		Pacific = 2,
		WPN = 3
	};

	/// <summary>
	/// Supported poker rooms
	/// </summary>
	enum class Rooms : int
	{
		Unrecognized = 0,
		PokerStars = 10,
		Pacific = 14,
		IPoker = 13,
		PartyPoker = 11,
		WinningPokerNetwork = 15,
		Winamax = 9,
		Microgaming = 7,
		Baazi = 1,
		CheckRaise = 2,
		Dollaro = 12,
		BetSense = 3,
		MonkeyBet = 4,
		Fulpot = 5,
		GGNet = 16,
		EnetPoker = 17,
		KlasPoker = 18,
		PokerWorld = 19,
		Ourgame = 20,
		BetOnline = 21,
		BIGBETGE = 22,
		BluffOnline = 23,
		BluffDaddy = 24,
		ColombiaPokerLive = 25,
		EuropeBetCom = 26,
		FTRPoker = 27,
		PokerGDFPLAY = 28,
		Highrollers = 29,
		ItalyLivePoker = 30,
		PokerMania = 31,
		PokerMIRA = 32,
		PokerDom = 33,
		Pokermatch = 34,
		RedArgentinaDePoker = 35,
		SekaBETCom = 36,
		Sekabet = 37,
		SpartanPokerCom = 38,
		SportsBetting = 39,
		TigerGaming = 40,
		VenezuelaPokerLive = 41,
		XMaster = 42,
		PokerGrant = 43,
		GrandPokerEu = 44,
		Revolutionbets = 45,
		Vbet = 46,
		Win2Day = 47,
		WWin = 48,
		PokerMaster = 49,
		PlanetWin365 = 50,
		AconcaguaPoker = 51,
		BrasilPokerLive = 52,
		SurPokerDeLasAmericas = 53,
		ChilePokerLive = 54,
		BoliviaPokerLive = 55,
		CostaRicaPokerLive = 56,
		GuaraniPokerLive = 57,
		MexicoPokerLive = 58,
		PeruPokerLive = 59,
		PPPoker = 60,
		PokerKingdom = 61,
		PokerKing = 62,
		FishPokers = 63,
		OhPoker = 64,
		OnePS = 65,
		PokerClans = 66,
		KKPoker = 67
	};

	/// <summary>
	/// Currencies supported by Hand2Note. Used in HandStart messages.
	/// </summary>
	enum class Currencies : int
	{
		Dollar = 1,
		Euro = 2,
		Pound = 3,
		PlayMoney = 4,
		Points = 5,
		Chips = 6,
		Yuan = 7,
		IndianRupee = 8,
		Hryvnia = 9,
		Rouble = 10,
		GeorgianLari = 11,
		Undefined = 0
	};

	/// <summary>
	/// Poker actions 
	/// </summary>
	enum class Actions : int
	{
		Fold = 0,
		Call = 1,
		Raise = 2,
		Check = 3,
		Bet = 4
	};

	/// <summary>
	/// Streets
	/// </summary>
	enum class Streets : int
	{
		Preflop = 0,
		Flop = 3,
		Turn = 4,
		River = 5
	};


	/// <summary>
	/// Hand2Note activity monitor. Contains methods for checking if Hand2Note is running, events for Hand2Note start and/or close.
	/// </summary>
	class ActivityMonitor 
	{
	public:
		
		/// <summary>
		/// Constructor
		/// </summary>
		/// <param name="immediateOnStart">if true and Hand2Note running, OnHand2NoteStart fired immediately</param>
		ActivityMonitor(bool immediateOnStart = true) : _immediateOnStart(immediateOnStart)
		{
			poll_thread_ = std::thread([this]() { Loop(); });
		}

		virtual ~ActivityMonitor() {
			{
				std::unique_lock<std::mutex> lock{ mutex_ };
				is_stop_ = true;
			}
			cvar_.notify_one();
			poll_thread_.join();
		}

		/// <summary>
		/// Checks if Hand2Note is running
		/// </summary>
		static inline bool IsHand2NoteRunning() {
			return h2n_is_running();
		}

		/// <summary>
		/// Time interval between IsHand2NoteRunning calls, used to fire <c>Hand2NoteStarted</c> and <c>OnHand2NoteClosed</c> events
		/// </summary>
		int PollDelay() const {
			return _pollDelay;
		}
		/// <summary>
		/// Time interval between IsHand2NoteRunning calls, used to fire <c>Hand2NoteStarted</c> and <c>OnHand2NoteClosed</c> events
		/// </summary>
		void PollDelay(int pollDelay) {
			_pollDelay = pollDelay;
		}


		/// <summary>
		/// Fired on when Hand2Note client is started
		/// </summary>
		/// <remarks>
		/// Fired immediately if ActivityMonitor was created with immediateOnStart == true
		/// </remarks>
		virtual void Hand2NoteStarted() {}
		/// <summary>
		/// Fired when Hand2Note client becomes closed
		/// </summary>
		virtual void Hand2NoteClosed() {}

	private:
		int _pollDelay{ 300 };
		std::thread poll_thread_;
		bool _immediateOnStart;
		bool _isRunning{ false };
		std::mutex mutex_;
		std::condition_variable cvar_;
		bool is_stop_{ false };

		void Loop()
		{
			_isRunning = IsHand2NoteRunning();
			if (_immediateOnStart && _isRunning)
			{
				Hand2NoteStarted();
			}
			for (;;) {
				{
					std::unique_lock<std::mutex> lock{ mutex_ };
					if (cvar_.wait_for(lock, std::chrono::milliseconds(_pollDelay), [this]() { return is_stop_; }))
						break;
				}
				auto currState = IsHand2NoteRunning();
				if (currState == _isRunning)
					continue;
				_isRunning = currState;
				if (_isRunning)
					Hand2NoteStarted();
				else
					Hand2NoteClosed();
			}
		}

	};

	/// <summary>
	/// Convert original Table name to format supported by Hand2Note
	/// </summary>
	/// <remarks>
	/// Hand2Note is built on the assumption that hand history contains all the information required to completely define a hand including its original Room. 
	/// Usually converted hand history doesn't define its original poker room. So, we use a prefix for a table name to store this information.
	/// Hand2Note reads target room from table name 'XXXXnnnnnnnn'
	/// where 'XXXX' is room prefix, 'nnnnnnnn' is some numeric hash code from original table name.
	/// </remarks>
	/// <param name="room">Original poker room</param>
	/// <param name="originalTableName">Original table name</param>
	/// <returns>Table name supported by Hand2Note</returns>
	static inline std::string GetRoomDefiningTableName(Rooms room, const std::string& originalTableName)
	{
		char *pTableName = h2n_make_table_name((int)room, originalTableName.c_str());
		std::string ret{ pTableName };
		h2n_free_cstring(pTableName);
		return ret;
	}


	/// <summary>
	/// Send message with completed (static) hand history text to Hand2Note. 
	/// </summary>
	class HandHistoryMessage
	{
	public:
		/// <summary>
		/// Target room
		/// </summary>
		Rooms Room() const {
			return room_;
		}
		/// <summary>
		/// Target room
		/// </summary>
		void Room(Rooms room) {
			room_ = room;
		}

		/// <summary>
		/// Game/Hand Index
		/// </summary>
		uint64_t GameNumber() const {
			return game_id_;
		}
		/// <summary>
		/// Game/Hand Index
		/// </summary>
		void GameNumber(uint64_t gameNumber) {
			game_id_ = gameNumber;
		}

		/// <summary>
		/// Is fastfold, zoom, boost table
		/// </summary>
		bool IsZoom() const {
			return is_zoom_;
		}
		void SetZoom(bool isZoom = true) {
			is_zoom_ = isZoom;
		}

		/// <summary>
		/// <c>HandHistory</c> text format, PS/888/WPN 
		/// </summary>
		HandHistoryFormats Format() const {
			return format_;
		}
		/// <summary>
		/// <c>HandHistory</c> text format, PS/888/WPN 
		/// </summary>
		void Format(HandHistoryFormats format) {
			format_ = format;
		}

		/// <summary>
		/// Completed hand history
		/// </summary>
		const std::string& HandHistory() const {
			return fhh_;
		}
		/// <summary>
		/// Completed hand history
		/// </summary>
		void HandHistory(const std::string& handHistory) {
			fhh_ = handHistory;
		}

		/// <summary>
		/// Original hand history text if there is any
		/// </summary>
		const std::string& OriginalHandHistory() const {
			return ohh_;
		}
		/// <summary>
		/// Original hand history text if there is any
		/// </summary>
		void OriginalHandHistory(const std::string& originalHandHistory) {
			ohh_ = originalHandHistory;
		}


		HandHistoryMessage() { }
	private:
		std::string fhh_;
		std::string ohh_;
		HandHistoryFormats format_{ HandHistoryFormats::Original };
		Rooms room_{ Rooms::Unrecognized };
		uint64_t game_id_{ 0 };
		bool is_zoom_{ false };
	};

	static inline int Send(const HandHistoryMessage& msg)
	{
		h2n_hh_message m;
		m.format = (int)msg.Format();
		m.gameid = (double)msg.GameNumber();
		m.hh_formatted = msg.HandHistory().c_str();
		m.hh_original = msg.OriginalHandHistory().c_str();
		m.is_zoom = msg.IsZoom() ? 1 : 0;
		m.room = (int)msg.Room();
		return h2n_send_handhistory(&m);
	}

	/// <summary>
	/// Seat data class, used in <c>HandStartMessage</c>. Contains nickname, index, stack, blinds etc
	/// </summary>
	class PlayerSeatInfo
	{
	public:
		/// <summary>
		/// Seat index in range [0..MaxPlayers-1]. SeatIndex == 0 if the seat is at 18 o'clock.
		/// </summary>
		int SeatIndex() const {
			return seat_idx_;
		}
		/// <summary>
		/// Seat index in range [0..MaxPlayers-1]. SeatIndex == 0 if the seat is at 18 o'clock.
		/// </summary>
		void SeatIndex(int seatIndex) {
			seat_idx_ = seatIndex;
		}

		/// <summary>
		/// Player nickname. On Chinese poker rooms this should be the visible nickname of the player, i.e. those one usually in Chinese letters.
		/// </summary>
		const std::string& Nickname() const {
			return nickname_;
		}
		/// <summary>
		/// Player nickname. On Chinese poker rooms this should be the visible nickname of the player, i.e. those one usually in Chinese letters.
		/// </summary>
		void Nickname(const std::string& nickname) {
			nickname_ = nickname;
		}

		/// <summary>
		/// Player ID used in china rooms, set it if possible or use hash code from nickname
		/// </summary>
		const std::string& PlayerShowId() const {
			return player_id_;
		}
		/// <summary>
		/// Player ID used in china rooms, set it if possible or use hash code from nickname
		/// </summary>
		void PlayerShowId(const std::string& playerId) {
			player_id_ = playerId;
		}


		/// <summary>
		/// Starting stack size, before blinds and ante
		/// </summary>
		double InitialStackSize() const {
			return stack_;
		}
		/// <summary>
		/// Starting stack size, before blinds and ante
		/// </summary>
		void InitialStackSize(double stack) {
			stack_ = stack;
		}

		/// <summary>
		/// Poket cards string in common format : '6sKcTdAh'. Can be empty even if <c>IsHero</c> set to true
		/// </summary>
		const std::string& PoketCards() const {
			return pocket_cards_;
		}
		/// <summary>
		/// Poket cards string in common format : '6sKcTdAh'. Can be empty even if <c>IsHero</c> set to true
		/// </summary>
		void PoketCards(const std::string& cards) {
			pocket_cards_ = cards;
		}

		/// <summary>
		/// Player is a dealer/button
		/// </summary>
		bool IsDealer() const {
			return is_dealer_;
		}
		/// <summary>
		/// Player is a dealer/button
		/// </summary>
		void SetDealer(bool isDealer = true) {
			is_dealer_ = isDealer;
		}


		/// <summary>
		/// Seat posted small blind
		/// </summary>
		bool IsPostedSmallBlind() const {
			return is_posted_sb_;
		}
		/// <summary>
		/// Seat posted small blind
		/// </summary>
		void SetPostedSmallBlind(bool isPostedSmallBlind = true) {
			is_posted_sb_ = isPostedSmallBlind;
		}

		/// <summary>
		/// Seat posted big blind
		/// </summary>
		bool IsPostedBigBlind() const {
			return is_posted_bb_;
		}
		/// <summary>
		/// Seat posted big blind
		/// </summary>
		void SetPostedBigBlind(bool isPostedBigBlind = true) {
			is_posted_bb_ = isPostedBigBlind;
		}

		/// <summary>
		/// Seat posted small blind out of queue, usually dead blind
		/// </summary>
		bool IsPostedSmallBlindOutOfQueue() const {
			return is_posted_sb_outofqueue_;
		}
		/// <summary>
		/// Seat posted small blind out of queue, usually dead blind
		/// </summary>
		void SetPostedSmallBlindOutOfQueue(bool isPostedSmallBlindOutOfQueue = true) {
			is_posted_sb_outofqueue_ = isPostedSmallBlindOutOfQueue;
		}

		/// <summary>
		/// Seat posted big blind out of queue, entry bet
		/// </summary>
		bool IsPostedBigBlindOutOfQueue() const {
			return is_posted_bb_outofqueue_;
		}
		/// <summary>
		/// Seat posted big blind out of queue, entry bet
		/// </summary>
		void SetPostedBigBlindOutOfQueue(bool isPostedBigBlindOutOfQueue = true) {
			is_posted_bb_outofqueue_ = isPostedBigBlindOutOfQueue;
		}

		/// <summary>
		/// Seat posted straddle
		/// </summary>
		bool IsPostedStraddle() const {
			return is_posted_straddle_;
		}
		/// <summary>
		/// Seat posted straddle
		/// </summary>
		void SetPostedStraddle(bool isPostedStraddle = true) {
			is_posted_straddle_ = isPostedStraddle;
		}

		/// <summary>
		/// Seat is hero, important flag used to arrange huds around preferred seat.
		/// </summary>
		bool IsHero() const {
			return is_hero_;
		}
		/// <summary>
		/// Seat is hero, important flag used to arrange huds around preferred seat.
		/// </summary>
		void SetHero(bool isHero = true) {
			is_hero_ = isHero;
		}

		/// <summary>
		/// Seat is sitting out
		/// </summary>
		bool IsSittingOut() const {
			return is_sitting_out_;
		}
		/// <summary>
		/// Seat is sitting out
		/// </summary>
		void SetSittingOut(bool isSittingOut = true) {
			is_sitting_out_ = isSittingOut;
		}

		PlayerSeatInfo() { }
	private:
		int          seat_idx_{ -1 };
		std::string  nickname_;
		std::string  player_id_;
		double       stack_{ 0 };
		std::string  pocket_cards_;
		bool         is_dealer_{ false };
		bool         is_posted_sb_{ false };
		bool         is_posted_bb_{ false };
		bool         is_posted_sb_outofqueue_{ false };
		bool         is_posted_bb_outofqueue_{ false };
		bool         is_posted_straddle_{ false };
		bool         is_hero_{ false };
		bool         is_sitting_out_{ false };
	};


	/// <summary>
	/// Send dynamic hand start message to Hand2Note
	/// </summary>
	/// <seealso cref="HandActionMessage"/>
	/// <seealso cref="HandDealMessage"/>
	class HandStartMessage
	{
	public:
		using SeatsList = std::vector<PlayerSeatInfo>;

		/// <summary>
		/// Target room
		/// </summary>
		Rooms Room() const {
			return room_;
		}
		/// <summary>
		/// Target room
		/// </summary>
		void Room(Rooms room) {
			room_ = room;
		}

		/// <summary>
		/// Game/Hand number
		/// </summary>
		uint64_t GameNumber() const {
			return game_id_;
		}
		/// <summary>
		/// Game/Hand number
		/// </summary>
		void GameNumber(uint64_t gameNumber) {
			game_id_ = gameNumber;
		}

		/// <summary>
		/// Table name in supported format, see <see cref="Hand2Note.GetRoomDefiningTableName(Rooms, string)"/>
		/// </summary>
		const std::string& TableName() const {
			return table_name_;
		}
		/// <summary>
		/// Table name in supported format, see <see cref="Hand2Note.GetRoomDefiningTableName(Rooms, string)"/>
		/// </summary>
		void TableName(const std::string& tableName) {
			table_name_ = tableName;
		}

		/// <summary>
		/// Table window handle (HWND) ( where to attach huds )
		/// </summary>
		int TableWindowHwnd() const {
			return table_hwnd_;
		}
		/// <summary>
		/// Table window handle (HWND) ( where to attach huds )
		/// </summary>
		void TableWindowHwnd(int hwnd) {
			table_hwnd_ = hwnd;
		}

		/// <summary>
		/// Table max players. 2-max, 6-max, 9-max etc
		/// </summary>
		int TableSize() const {
			return max_players_;
		}
		/// <summary>
		/// Table max players. 2-max, 6-max, 9-max etc
		/// </summary>
		void TableSize(int tableSize) {
			max_players_ = tableSize;
		}

		/// <summary>
		/// Is it tournament hand 
		/// </summary>
		bool IsTourney() const {
			return is_tourney_;
		}
		/// <summary>
		/// Is it tournament hand 
		/// </summary>
		void SetTourney(bool isTourney) {
			is_tourney_ = isTourney;
		}

		/// <summary>
		/// Is it omaha hand
		/// </summary>
		bool IsOmaha() const {
			return is_omaha_;
		}
		/// <summary>
		/// Is it omaha hand
		/// </summary>
		void SetOmaha(bool isOmaha) {
			is_omaha_ = isOmaha;
		}

		/// <summary>
		/// Is it fixed limit game
		/// </summary>
		bool IsLimit() const {
			return is_limit_;
		}
		/// <summary>
		/// Is it fixed limit game
		/// </summary>
		void SetLimit(bool isLimit) {
			is_limit_ = isLimit;
		}

		/// <summary>
		/// Is it zoom/fastfold/boost hand
		/// </summary>
		bool IsZoom() const {
			return is_zoom_;
		}
		/// <summary>
		/// Is it zoom/fastfold/boost hand
		/// </summary>
		void SetZoom(bool isZoom) {
			is_zoom_ = isZoom;
		}

		/// <summary>
		/// Is there a cap 
		/// </summary>
		bool IsCap() const {
			return is_cap_;
		}
		/// <summary>
		/// Is there a cap 
		/// </summary>
		void SetCap(bool isCap) {
			is_cap_ = isCap;
		}

		/// <summary>
		/// Is it pot limit game
		/// </summary>
		bool IsPotLimit() const {
			return is_potlimit_;
		}
		/// <summary>
		/// Is it pot limit game
		/// </summary>
		void SetPotLimit(bool isPotLimit) {
			is_potlimit_ = isPotLimit;
		}

		/// <summary>
		/// Room currency
		/// </summary>
		Currencies Currency() const {
			return currency_;
		}
		/// <summary>
		/// Room currency
		/// </summary>
		void Currency(Currencies currency) {
			currency_  = currency;
		}

		/// <summary>
		/// Small blind amount
		/// </summary>
		double SmallBlind() const {
			return sb_;
		}
		/// <summary>
		/// Small blind amount
		/// </summary>
		void SmallBlind(double sb) {
			sb_ = sb;
		}

		/// <summary>
		/// Big blind amount
		/// </summary>
		double BigBlind() const {
			return bb_;
		}
		/// <summary>
		/// Big blind amount
		/// </summary>
		void BigBlind(double bb) {
			bb_ = bb;
		}

		/// <summary>
		/// Ante amount
		/// </summary>
		double Ante() const {
			return ante_;
		}
		/// <summary>
		/// Ante amount
		/// </summary>
		void Ante(double ante) {
			ante_ = ante;
		}

		/// <summary>
		/// Straddle amount
		/// </summary>
		double Straddle() const {
			return straddle_;
		}
		/// <summary>
		/// Straddle amount
		/// </summary>
		void Straddle(double straddle) {
			straddle_ = straddle;
		}

		/// <summary>
		/// List of table seats.
		/// </summary>
		SeatsList& Seats() {
			return seats_;
		}
		/// <summary>
		/// List of table seats.
		/// </summary>
		const SeatsList& Seats() const {
			return seats_;
		}

		HandStartMessage() { }

	private:
		Rooms       room_{ Rooms::Unrecognized };
		uint64_t    game_id_{ 0 };
		std::string table_name_;
		int         table_hwnd_{ 0 };
		int         max_players_{ 9 };
		bool        is_tourney_{ false };
		bool        is_omaha_{ false };
		bool        is_limit_{ false };
		bool        is_zoom_{ false };
		bool        is_cap_{ false };
		bool        is_potlimit_{ false };
		Currencies  currency_{ Currencies::Dollar };
		double      sb_{ 0 };
		double      bb_{ 0 };
		double      ante_{ 0 };
		double      straddle_{ 0 };
		SeatsList   seats_;
	};

	static inline int Send(const HandStartMessage& msg)
	{
		h2n_start_hand_message m;
		m.ante = msg.Ante();
		m.bb = msg.BigBlind();
		m.currency = (int)msg.Currency();
		m.gameid = (double)msg.GameNumber();
		m.is_cap = msg.IsCap() ? 1 : 0;
		m.is_limit = msg.IsLimit() ? 1 : 0;
		m.is_omaha = msg.IsOmaha() ? 1 : 0;
		m.is_potlimit = msg.IsPotLimit() ? 1 : 0;
		m.is_tourney = msg.IsTourney() ? 1 : 0;
		m.is_zoom = msg.IsZoom() ? 1 : 0;
		m.max_players = msg.TableSize();
		m.room = (int)msg.Room();
		m.sb = msg.SmallBlind();
		m.straddle = msg.Straddle();
		m.table_hwnd = msg.TableWindowHwnd();
		m.table_name = msg.TableName().c_str();

		m.seats_num = 0;
		for (HandStartMessage::SeatsList::const_iterator it =  msg.Seats().cbegin(); it != msg.Seats().cend(); ++it) {
			const PlayerSeatInfo& seat = *it;
			h2n_seat_info *s = &(m.seats[m.seats_num++]);

			s->is_dealer = seat.IsDealer() ? 1 : 0;
			s->is_hero = seat.IsHero() ? 1 : 0;
			s->is_posted_bb = seat.IsPostedBigBlind() ? 1 : 0;
			s->is_posted_bb_outofqueue = seat.IsPostedBigBlindOutOfQueue()  ? 1 : 0;
			s->is_posted_sb = seat.IsPostedSmallBlind() ? 1 : 0;
			s->is_posted_sb_outofqueue = seat.IsPostedSmallBlindOutOfQueue()  ? 1 : 0;
			s->is_posted_straddle = seat.IsPostedStraddle() ? 1 : 0;
			s->is_sitting_out = seat.IsSittingOut() ? 1 : 0;
			s->nickname = seat.Nickname().c_str();
			s->player_id = seat.PlayerShowId().c_str();
			s->pocket_cards = seat.PoketCards().c_str();
			s->seat_idx = seat.SeatIndex();
			s->stack = seat.InitialStackSize();
		}
		
		return h2n_send_hand_start(&m);
	}



	/// <summary>
	/// Send dynamic poker action message to Hand2Note, used with <see cref="HandDealMessage"/> to maintain dynamic deal state after <see cref="HandStartMessage"/>
	/// </summary>
	class HandActionMessage
	{
	public:
		/// <summary>
		/// Hand game number
		/// </summary>
		uint64_t GameNumber() const {
			return game_id_;
		}
		/// <summary>
		/// Hand game number
		/// </summary>
		void GameNumber(uint64_t gameNumber) {
			game_id_ = gameNumber;
		}

		/// <summary>
		/// Action seat index in range [0..MaxPlayers-1]
		/// </summary>
		int SeatIndex() const {
			return seat_idx_;
		}
		/// <summary>
		/// Action seat index in range [0..MaxPlayers-1]
		/// </summary>
		void SeatIndex(int seatIndex) {
			seat_idx_ = seatIndex;
		}

		/// <summary>
		/// Action type. Fold/Check/Raise etc
		/// </summary>
		Actions ActionType() const {
			return type_;
		}
		/// <summary>
		/// Action type. Fold/Check/Raise etc
		/// </summary>
		void ActionType(Actions actionType) {
			type_ = actionType;
		}

		/// <summary>
		/// Call/Bet/Raise amount. If the ActionType is raise then Amount should contain the value "raised to", i.e. the current bet amount of the player .
		/// </summary>
		double Amount() const {
			return amount_;
		}
		/// <summary>
		/// Call/Bet/Raise amount. If the ActionType is raise then Amount should contain the value "raised to", i.e. the current bet amount of the player .
		/// </summary>
		void Amount(double amount) {
			amount_ = amount;
		}

		HandActionMessage() { }
	private:
		uint64_t    game_id_{ 0 };
		int         seat_idx_{ -1 };
		Actions     type_{ Actions::Fold };
		double      amount_{ 0 };
	};

	static inline int Send(const HandActionMessage& msg)
	{
		h2n_action_message m;
		m.amount = msg.Amount();
		m.gameid = (double)msg.GameNumber();
		m.is_allin = 0;
		m.pot = 0;
		m.seat_idx = msg.SeatIndex();
		m.type = (int)msg.ActionType();
		return h2n_send_action(&m);
	}


	/// <summary>
	/// Send dynamic poker street message to Hand2Note, used with <see cref="HandActionMessage"/> to maintain dynamic deal state after <see cref="HandStartMessage"/>
	/// </summary>
	class HandDealMessage
	{
	public:
		/// <summary>
		/// Game/Hand number
		/// </summary>
		uint64_t GameNumber() const {
			return game_id_;
		}
		/// <summary>
		/// Game/Hand number
		/// </summary>
		void GameNumber(uint64_t gameNumber) {
			game_id_ = gameNumber;
		}

		/// <summary>
		/// Street type. Flop/Turn/River
		/// </summary>
		Streets Street() const {
			return type_;
		}
		/// <summary>
		/// Street type. Flop/Turn/River
		/// </summary>
		void Street(Streets street) {
			type_ = street;
		}

		/// <summary>
		/// Board cards string in common format '5cQdTh'. 3 cards for flop, 4 for turn, 5 for river
		/// </summary>
		const std::string& Board() const {
			return board_;
		}
		/// <summary>
		/// Board cards string in common format '5cQdTh'. 3 cards for flop, 4 for turn, 5 for river
		/// </summary>
		void Board(const std::string& board) {
			board_ = board;
		}

		/// <summary>
		/// Total pot value on street. Is not required for hands without rake.
		/// </summary>
		double Pot() const {
			return pot_;
		}
		/// <summary>
		/// Total pot value on street. Is not required for hands without rake.
		/// </summary>
		void Pot(double pot) {
			pot_ = pot;
		}

		HandDealMessage() { }
	private:
		uint64_t    game_id_{ 0 };
		Streets     type_{ Streets::Preflop };
		std::string board_;
		double      pot_{ 0 };
	};

	static inline int Send(const HandDealMessage& msg)
	{
		h2n_street_message m;
		m.board = msg.Board().c_str();
		m.gameid = (double)msg.GameNumber();
		m.pot = msg.Pot();
		m.type = (int)msg.Street();
		return h2n_send_street(&m);
	}

}


#endif
