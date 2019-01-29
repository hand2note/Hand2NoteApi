#include "catch.hpp"
#include "h2napi.hpp"

#include <windows.h>
#include <regex>
#include <random>


TEST_CASE("TestActivityMonitor")
{

	// IsHand2NoteRunning static method
	CHECK((FindWindowW(NULL, L"Hand2Note 2") != 0) == Hand2Note::ActivityMonitor::IsHand2NoteRunning());


	// override events class
	class MyActivityMonitor : public Hand2Note::ActivityMonitor {
	public:
		using ActivityMonitor::ActivityMonitor;

		void Hand2NoteStarted() override {
			INFO("Hand2NoteStarted Fired");
			IsHand2NoteStartedFired = true;
		}

		void Hand2NoteClosed() override {
			INFO("Hand2NoteClosed Fired");
		}

		std::atomic_bool IsHand2NoteStartedFired{ false };
	};

	
	bool IsSendOnStarImmediate = true; // or false

	MyActivityMonitor monitor(IsSendOnStarImmediate);

	// you can change poll delay for ActivityMonitor (time interval between IsHand2NoteRunning checks) 
	CHECK(300 == monitor.PollDelay());
	monitor.PollDelay(100);


	if (MyActivityMonitor::IsHand2NoteRunning())
	{
		// if h2n is running and  IsSendOnStarImmediate is set to true we expect OnHand2NoteStart to be fired almost immediately
		for(int i = 0; i < 5 && !monitor.IsHand2NoteStartedFired; ++i)
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

		CHECK( IsSendOnStarImmediate == monitor.IsHand2NoteStartedFired );

		// close h2n manually and wait some time for ActivityMonitor::OnHand2NoteStop 
		// std::this_thread::sleep_for(std::chrono::milliseconds(10000));
	}

}


TEST_CASE("TestH2NTableName")
{
	// hand2note only accept handhistories and dynamic data with XXXXnnnnnnn table name format, where XXXX is room prefix, nnnnnnn - some hash from original table name
	// to convert original table name to format supported by h2n use [Hand2Note::GetRoomDefiningTableName]

	auto s1 = Hand2Note::GetRoomDefiningTableName(Hand2Note::Rooms::FishPokers, "some table name");
	CHECK( std::regex_match(s1, std::regex("^FSHP[0-9]+$")) );

	auto s2 = Hand2Note::GetRoomDefiningTableName(Hand2Note::Rooms::FishPokers, "some table name");
	//// same original table name - same output
	CHECK(s1 == s2);

	auto s3 = Hand2Note::GetRoomDefiningTableName(Hand2Note::Rooms::FishPokers, "table name");
	CHECK(s1 != s3);


	auto s4 = Hand2Note::GetRoomDefiningTableName(Hand2Note::Rooms::PokerStars, "table name");
	// same original table name but different prefix
	CHECK(s3 != s4);

	// prefix depends on room
	CHECK(std::regex_match(s4, std::regex("^PS[0-9]+$")));

}

std::mt19937& _mt_gen() {
	static std::random_device rd_;
	static std::mt19937 mt_gen_(rd_());
	return mt_gen_;
}

template<class T> T my_rand(T _min0, T _max0) {
	return std::uniform_int_distribution<T>(_min0, _max0)(_mt_gen());
}


TEST_CASE("TestSendCompletedHandHistory")
{
	// Hand2Note api requires UTF8 strings
        static const std::string PokerFishFormattedHandHistory = u8R"(PokerStars Hand #2416948123: Hold'em No Limit ($0.25/$0.50 USD) - 2019/01/21 13:44:57 ET
Table '大安上王33' 9-max Seat #7 is the button
Seat 1: 无能为力 ($109.54 in chips)
Seat 3: 张琳 ($168.45 in chips)
Seat 4: 天天大水上 ($59.26 in chips)
Seat 5: 安排！ ($48.14 in chips)
Seat 7: 木樽 ($247.19 in chips)
Seat 8: 德州小丑王 ($53.82 in chips)
德州小丑王: posts the ante $0.25
无能为力: posts the ante $0.25
张琳: posts the ante $0.25
天天大水上: posts the ante $0.25
安排！: posts the ante $0.25
木樽: posts the ante $0.25
德州小丑王: posts small blind $0.25
无能为力: posts big blind $0.50
*** HOLE CARDS ***
张琳: raises $0.50 to $1
天天大水上: folds
安排！: folds
木樽: folds
德州小丑王: calls $0.75
无能为力: folds
*** FLOP *** [5h 8s 7s]
德州小丑王: checks
张琳: bets $2.66
德州小丑王: calls $2.66
*** TURN *** [5h 8s 7s] [Ts]
德州小丑王: checks
张琳: bets $6.21
德州小丑王: folds
Uncalled bet ($6.21) returned to 张琳
张琳 collected $8.82 from pot
张琳: doesn't show hand
*** SUMMARY ***
Total pot $8.82 | Rake $0
Board [5h 8s 7s Ts]
Seat 1: 无能为力 (big blind) folded before Flop
Seat 3: 张琳 collected ($9.32)
Seat 4: 天天大水上 folded before Flop (didn't bet)
Seat 5: 安排！ folded before Flop (didn't bet)
Seat 7: 木樽 (button) folded before Flop (didn't bet)
Seat 8: 德州小丑王 (small blind) folded on the Turn
)";

		
		// we dont need to check is h2n running or not, messages are written into internal ipc circle buffer
		// if h2n is running its gonna grab em from ipc queue and process

		// --------------------------------------------------------------------------------------------------------------
		// need to convert table name 

		// get original table name
		std::smatch match;
		CHECK(std::regex_search(PokerFishFormattedHandHistory, match, std::regex("Table '(.+)'")));

		auto tableName = match[1].str();
		auto realName = u8"大安上王33";
		CHECK(realName == tableName);

		// make table name required by h2n
		auto h2nTableName = Hand2Note::GetRoomDefiningTableName(Hand2Note::Rooms::FishPokers, tableName);
		REQUIRE(std::regex_match(h2nTableName, std::regex("^FSHP[0-9]+$")));

		// replace
		auto HandHistory = std::regex_replace(PokerFishFormattedHandHistory, std::regex(tableName), h2nTableName);
		
		// --------------------------------------------------------------------------------------------------------------
		// create hand history message for hand2note

		Hand2Note::HandHistoryMessage msg;

		// hand history format is pokerstars
		msg.Format ( Hand2Note::HandHistoryFormats::Stars );

		// next params required by h2n for quick search / reject dups w/o parsing handhistory
		// room is pokerfishS
		msg.Room ( Hand2Note::Rooms::FishPokers );
		// game id / hand id
		REQUIRE(std::regex_search(HandHistory, match, std::regex("Hand #([0-9]+):")));
		msg.GameNumber ( atoll(match[1].str().c_str()) );
		CHECK(msg.GameNumber() == 2416948123);
		// lets randomize it for testing
		auto newGameId = my_rand(INT_MAX / 2, INT_MAX);
		HandHistory = std::regex_replace(HandHistory, std::regex(match[1].str()), std::to_string(newGameId));
		msg.GameNumber ( newGameId );

		// IsZoom == false by default
		CHECK_FALSE(msg.IsZoom());

		// handhistory in msg.Format (pokerstars)
		msg.HandHistory ( HandHistory );

		// original handhistory is internal room format, for example xml for connective
		// for pokerfish there is no original format
		CHECK(msg.OriginalHandHistory().empty());


		// send message, check h2n smart inspect (.sil) log for
		// 'Integrated Hh #XXXX = "PokerStars Hand #XXXX ...' message
		CHECK( Hand2Note::Send(msg) == 0 );


}

TEST_CASE("TestSendDynamicHandHistory")
{
	// Dynamic hand template
	// ----------------------------------------------------------------------------------------------------
	//PokerStars Hand #2416948123: Hold'em No Limit ($0.25/$0.50 USD) - 2019/01/21 13:44:57 ET
	//Table '大安上王33' 9-max Seat #7 is the button
	//Seat 1: 无能为力 ($109.54 in chips)
	//Seat 3: 张琳 ($168.45 in chips)
	//Seat 4: 天天大水上 ($59.26 in chips)
	//Seat 5: 安排！ ($48.14 in chips)
	//Seat 7: 木樽 ($247.19 in chips)
	//Seat 8: 德州小丑王 ($53.82 in chips)
	//德州小丑王: posts the ante $0.25
	//无能为力: posts the ante $0.25
	//张琳: posts the ante $0.25
	//天天大水上: posts the ante $0.25
	//安排！: posts the ante $0.25
	//木樽: posts the ante $0.25
	//德州小丑王: posts small blind $0.25
	//无能为力: posts big blind $0.50
	//*** HOLE CARDS ***
	//张琳: raises $0.50 to $1
	//天天大水上: folds
	//安排！: folds
	//木樽: folds
	//德州小丑王: calls $0.75
	//无能为力: folds
	//*** FLOP *** [5h 8s 7s]
	//德州小丑王: checks
	//张琳: bets $2.66
	//德州小丑王: calls $2.66
	//*** TURN *** [5h 8s 7s] [Ts]
	//德州小丑王: checks
	//张琳: bets $6.21
	//德州小丑王: folds
	// ----------------------------------------------------------------------------------------------------

	Hand2Note::HandStartMessage msg;

	msg.Ante ( 0.25 );
	msg.SmallBlind ( 0.25 );
	msg.BigBlind ( 0.5 );
	msg.Currency (Hand2Note::Currencies::Yuan );
	// real gameId is 2416948123, but we'll use random for testing
	msg.GameNumber ( my_rand(INT_MAX / 2, INT_MAX) );
	msg.Room (Hand2Note::Rooms::FishPokers );
	msg.TableSize ( 9 );
	// need table name in h2n format, XXXXnnnn
	auto realName = u8"大安上王33";
	msg.TableName ( Hand2Note::GetRoomDefiningTableName(Hand2Note::Rooms::FishPokers, realName) );
	CHECK(std::regex_match(msg.TableName(), std::regex("^FSHP[0-9]+$")));

	// check default values...
	CHECK(msg.Straddle() == 0); // always set straddle if there is any
	CHECK_FALSE(msg.IsCap());
	CHECK_FALSE(msg.IsLimit());
	CHECK_FALSE(msg.IsOmaha());
	CHECK_FALSE(msg.IsPotLimit());
	CHECK_FALSE(msg.IsTourney());
	CHECK_FALSE(msg.IsZoom());


	static const auto HASHF = std::hash<std::string>();

	// there are 6 seats in template
	// -- seat 1 -------------------------------------------------------
	Hand2Note::PlayerSeatInfo s1;
	s1.Nickname ( u8"无能为力" );
	// player id used in china rooms, fill it with hash if you cant get real playerId
	s1.PlayerShowId ( std::to_string(HASHF(s1.Nickname()) & 0xFFFFFFFF) );
	// h2n seat indexes are in [0..MaxPlayers-1]
	s1.SeatIndex ( 0 ); // 1
	s1.InitialStackSize( 109.54 );
	// seat 1 posted big blind
	s1.SetPostedBigBlind();
	// everything else is false by default
	CHECK_FALSE( (s1.IsDealer() || s1.IsHero() || s1.IsPostedSmallBlind() ||
		s1.IsPostedStraddle() || s1.IsPostedSmallBlindOutOfQueue() || s1.IsPostedBigBlindOutOfQueue() || s1.IsSittingOut()) );
	// no poket cards
	CHECK(s1.PoketCards().empty());
	msg.Seats().emplace_back(s1);

	// -- seat 3 -------------------------------------------------------
	Hand2Note::PlayerSeatInfo s3;
	s3.Nickname ( u8"张琳" );
	s3.PlayerShowId(std::to_string(HASHF(s3.Nickname()) & 0xFFFFFFFF));
	s3.SeatIndex(2); // 3
	s3.InitialStackSize(168.45);
	CHECK_FALSE( (s3.IsDealer() || s3.IsHero() || s3.IsPostedSmallBlind() || s3.IsPostedBigBlind() ||
		s3.IsPostedStraddle() || s3.IsPostedSmallBlindOutOfQueue() || s3.IsPostedBigBlindOutOfQueue() ||
		s3.IsSittingOut() ));
	CHECK(s3.PoketCards().empty());
	msg.Seats().emplace_back(s3);

	// -- seat 4 -------------------------------------------------------
	Hand2Note::PlayerSeatInfo s4;
	s4.Nickname ( u8"天天大水上" );
	s4.PlayerShowId(std::to_string(HASHF(s4.Nickname()) & 0xFFFFFFFF));
	s4.SeatIndex( 3); // 4
	s4.InitialStackSize (59.26);
	CHECK_FALSE( (s4.IsDealer() || s4.IsHero() || s4.IsPostedSmallBlind() || s4.IsPostedBigBlind() ||
		s4.IsPostedStraddle() || s4.IsPostedSmallBlindOutOfQueue() || s4.IsPostedBigBlindOutOfQueue() ||
		s4.IsSittingOut()));
	CHECK(s4.PoketCards().empty());
	msg.Seats().emplace_back(s4);


	// -- seat 5 -------------------------------------------------------
	Hand2Note::PlayerSeatInfo s5;
	s5.Nickname(u8"安排！");
	s5.PlayerShowId(std::to_string(HASHF(s5.Nickname()) & 0xFFFFFFFF));
	s5.SeatIndex(4); // 5
	s5.InitialStackSize (48.14);
	CHECK_FALSE( (s5.IsDealer() || s5.IsHero() || s5.IsPostedSmallBlind() || s5.IsPostedBigBlind() ||
		s5.IsPostedStraddle() || s5.IsPostedSmallBlindOutOfQueue() || s5.IsPostedBigBlindOutOfQueue() ||
		s5.IsSittingOut()));
	CHECK(s5.PoketCards().empty());
	msg.Seats().emplace_back(s5);

	// -- seat 7 -------------------------------------------------------
	Hand2Note::PlayerSeatInfo s7;
	s7.Nickname ( u8"木樽" );
	s7.PlayerShowId(std::to_string(HASHF(s7.Nickname()) & 0xFFFFFFFF));
	s7.SeatIndex (6); // 7
	s7.InitialStackSize (247.19);
	// seat 7 is dealer
	s7.SetDealer();
	CHECK_FALSE((s7.IsHero() || s7.IsPostedSmallBlind() || s7.IsPostedBigBlind() ||
		s7.IsPostedStraddle() || s7.IsPostedSmallBlindOutOfQueue() || s7.IsPostedBigBlindOutOfQueue() ||
		s7.IsSittingOut()));
	CHECK(s7.PoketCards().empty());
	msg.Seats().emplace_back(s7);

	// -- seat 8 -------------------------------------------------------
	Hand2Note::PlayerSeatInfo s8;
	s8.Nickname ( u8"德州小丑王" );
	s8.PlayerShowId(std::to_string(HASHF(s8.Nickname()) & 0xFFFFFFFF));
	s8.SeatIndex ( 7); // 8
	s8.InitialStackSize ( 53.82);
	// seat 8 posted small blind
	s8.SetPostedSmallBlind();
	// lets say its hero
	s8.SetHero();
	CHECK_FALSE( (s8.IsDealer() || s8.IsPostedBigBlind() ||
		s8.IsPostedStraddle() || s8.IsPostedSmallBlindOutOfQueue() || s8.IsPostedBigBlindOutOfQueue() ||
		s8.IsSittingOut()));
	// is hero flag can be used with or without poket cards, h2n uses IsHero to arrange huds
	CHECK(s8.PoketCards().empty());
	msg.Seats().emplace_back(s8);

	CHECK(6 == msg.Seats().size());

	// -------------------------------------------------------------------
	// ok we need to assign 'hand start' message to some window via HWND
	// open notepad, find notepad window handle with spy++ and setup msg.TableWindowHwnd
	msg.TableWindowHwnd ( 0x00B1235C );
	if (!IsWindow((HWND)msg.TableWindowHwnd()))
	{
		INFO("HandStart Message requires valid window handle");
	}
	REQUIRE(IsWindow((HWND)msg.TableWindowHwnd()));

	// send message, h2n huds should appear on notepad window
	CHECK( 0 == Hand2Note::Send(msg) );

	auto DELAY = 500u;

	// 张琳: raises $0.50 to $1
	Hand2Note::HandActionMessage a1_msg;
	a1_msg.GameNumber ( msg.GameNumber() );
	a1_msg.ActionType (Hand2Note::Actions::Raise);
	a1_msg.Amount ( 0.5 );
	a1_msg.SeatIndex ( 2 );
	Hand2Note::Send(a1_msg);

	Sleep(DELAY);

	// 天天大水上: folds
	Hand2Note::HandActionMessage a2_msg;
	a2_msg.GameNumber ( msg.GameNumber() );
	a2_msg.ActionType (Hand2Note::Actions::Fold);
	CHECK(0 == a2_msg.Amount() );
	a2_msg.SeatIndex ( 3 );
	Hand2Note::Send(a2_msg);

	Sleep(DELAY);

	//安排！: folds
	Hand2Note::HandActionMessage a3_msg;
	a3_msg.GameNumber ( msg.GameNumber() );
	a3_msg.ActionType (Hand2Note::Actions::Fold );
	CHECK( 0 == a3_msg.Amount() );
	a3_msg.SeatIndex ( 4 );
	Hand2Note::Send(a3_msg);

	Sleep(DELAY);

	//木樽: folds
	Hand2Note::HandActionMessage a4_msg;
	a4_msg.GameNumber ( msg.GameNumber() );
	a4_msg.ActionType (Hand2Note::Actions::Fold );
	CHECK(0 == a4_msg.Amount());
	a4_msg.SeatIndex ( 6 );
	Hand2Note::Send(a4_msg);

	Sleep(DELAY);

	//德州小丑王: calls $0.75
	Hand2Note::HandActionMessage a5_msg;
	a5_msg.GameNumber ( msg.GameNumber() );
	a5_msg.ActionType (Hand2Note::Actions::Call);
	a5_msg.Amount ( 0.75 );
	a5_msg.SeatIndex ( 7 );
	Hand2Note::Send(a5_msg);

	Sleep(DELAY);

	//无能为力: folds
	Hand2Note::HandActionMessage a6_msg;
	a6_msg.GameNumber ( msg.GameNumber() );
	a6_msg.ActionType (Hand2Note::Actions::Fold );
	CHECK(0 == a6_msg.Amount());
	a6_msg.SeatIndex ( 0 );
	Hand2Note::Send(a6_msg);


	Sleep(DELAY);

	// POT = 3.50
	//*** FLOP *** [5h 8s 7s]
	Hand2Note::HandDealMessage flop_msg;
	flop_msg.GameNumber ( msg.GameNumber() );
	flop_msg.Board ( "5h8s7s" );
	flop_msg.Street (Hand2Note::Streets::Flop);
	flop_msg.Pot ( 3.50 );
	Hand2Note::Send(flop_msg);


	Sleep(DELAY);

	//德州小丑王: checks
	Hand2Note::HandActionMessage a7_msg;
	a7_msg.GameNumber ( msg.GameNumber() );
	a7_msg.ActionType (Hand2Note::Actions::Check );
	CHECK(0 == a7_msg.Amount());
	a7_msg.SeatIndex ( 7 );
	Hand2Note::Send(a7_msg);

	Sleep(DELAY);

	//张琳: bets $2.66
	Hand2Note::HandActionMessage a8_msg;
	a8_msg.GameNumber ( msg.GameNumber() );
	a8_msg.ActionType (Hand2Note::Actions::Bet);
	a8_msg.Amount ( 2.66 );
	a8_msg.SeatIndex ( 2 );
	Hand2Note::Send(a8_msg);

	Sleep(DELAY);

	//德州小丑王: calls $2.66
	Hand2Note::HandActionMessage a9_msg;
	a9_msg.GameNumber( msg.GameNumber() );
	a9_msg.ActionType(Hand2Note::Actions::Call);
	a9_msg.Amount ( 2.66 );
	a9_msg.SeatIndex ( 7 );
	Hand2Note::Send(a9_msg);

	Sleep(DELAY);

	// POT = 8.82
	//*** TURN *** [5h 8s 7s] [Ts]
	Hand2Note::HandDealMessage turn_msg;
	turn_msg.GameNumber ( msg.GameNumber() );
	turn_msg.Board ( "5h8s7sTs" );
	turn_msg.Street (Hand2Note::Streets::Turn);
	turn_msg.Pot ( 8.82 );
	Hand2Note::Send(turn_msg);

	Sleep(DELAY);

	//德州小丑王: checks
	Hand2Note::HandActionMessage a10_msg;
	a10_msg.GameNumber ( msg.GameNumber() );
	a10_msg.ActionType (Hand2Note::Actions::Check );
	CHECK( 0 == a10_msg.Amount());
	a10_msg.SeatIndex ( 7 );
	Hand2Note::Send(a10_msg);

	Sleep(DELAY);

	// 张琳: bets $6.21
	Hand2Note::HandActionMessage a11_msg;
	a11_msg.GameNumber ( msg.GameNumber() );
	a11_msg.ActionType (Hand2Note::Actions::Bet );
	a11_msg.Amount ( 6.21 );
	a11_msg.SeatIndex ( 2 );
	Hand2Note::Send(a11_msg);

	Sleep(DELAY);

	// 德州小丑王: folds
	Hand2Note::HandActionMessage a12_msg;
	a12_msg.GameNumber ( msg.GameNumber() );
	a12_msg.ActionType (Hand2Note::Actions::Fold );
	CHECK(0 == a12_msg.Amount());
	a12_msg.SeatIndex ( 7 );
	Hand2Note::Send(a12_msg);

}