using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;

namespace Hand2Note.Api.Tests
{
    public static class FileSysHelpers
    {
        public static string Hand2NoteApiRoot { get; }  = Directory
            .GetParent(Assembly.GetExecutingAssembly().Location)
            .Parent.Parent.Parent.Parent.Parent.FullName;
        public static string Hand2NoteApiDLLPath { get; } =
             $"{Hand2NoteApiRoot}\\bin\\{(Environment.Is64BitOperatingSystem ? "x64" : "x86")}\\h2napi.dll";
    }

    public static class WinApiHelper
    {
        [DllImport("user32.dll", CharSet = CharSet.Unicode, EntryPoint = "FindWindowW")]
        public static extern int FindWindow(string sClass, string sWindow);
        [DllImport("user32.dll")]
        public static extern int IsWindow(IntPtr hwnd);
    }

    [TestClass]
    public class H2NApiDLLTest
    {
        public H2NApiDLLTest()
        {
            // free dll before test cases
            Hand2Note.FreeLibrary();
            Hand2Note.DLLPath = "h2napi.dll";
        }

        [TestMethod]
        public void TestDllLoad()
        {
            // by default DLLPath is 'h2napi.dll' i.e. current directory
            Assert.AreEqual("h2napi.dll", Hand2Note.DLLPath);

            // Hand2Note.Api.dll using lazy init strategy for loading h2napi.dll
            // call some method to force loadlibrary, we expect exception to be thrown, if no h2napi.dll in current path
            Assert.ThrowsException<InvalidOperationException> (() => ActivityMonitor.IsHand2NoteRunning );

            // set good path
            Hand2Note.DLLPath = FileSysHelpers.Hand2NoteApiDLLPath;

            // call method again, after that, dll should be loaded
            Assert.AreEqual(true, ActivityMonitor.IsHand2NoteRunning || true);

            // free dll, returns true if dll was successfully closed, we expect true, dll has been loaded after ActivityMonitor.IsHand2NoteRunning{get;}
            Assert.IsTrue(Hand2Note.FreeLibrary());

            // expect false if we try to free library again
            Assert.IsFalse(Hand2Note.FreeLibrary());
        }
    }

    [TestClass]
    public class H2NActivityMonitorTest
    {
        public H2NActivityMonitorTest()
        {
            Hand2Note.DLLPath = FileSysHelpers.Hand2NoteApiDLLPath;
            // free dll before test cases
            Hand2Note.FreeLibrary();
        }

        [TestMethod]
        public void TestIsRunning()
        {
            // IsHand2NoteRunning static method
            Assert.AreEqual(WinApiHelper.FindWindow(null, "Hand2Note 2") != 0, ActivityMonitor.IsHand2NoteRunning);
        }

        private ManualResetEvent _waitH2NStart = new ManualResetEvent(false);

        private void H2NStarted()
        {
            Console.WriteLine("ActivityMonitor::OnHand2NoteStart fired");
            _waitH2NStart.Set();
        }

        private void H2NStop()
        {
            Console.WriteLine("ActivityMonitor::OnHand2NoteStop fired");
        }

        [TestMethod]
        public void EventsTest()
        {
            bool IsSendOnStarImmediate = true; // or false

            // create monitor
            var monitor = new ActivityMonitor(IsSendOnStarImmediate);

            // assign events
            monitor.Hand2NoteStarted += H2NStarted;
            monitor.Hand2NoteClosed += H2NStop;

            // you can change poll delay for ActivityMonitor (time interval between IsHand2NoteRunning checks) 
            Assert.AreEqual(300, monitor.PollDelayMs); // 300ms by default
            monitor.PollDelayMs = 100;

            if (ActivityMonitor.IsHand2NoteRunning)
            {
                // if h2n is running and  IsSendOnStarImmediate is set to true we expect OnHand2NoteStart to be fired almost immediately
                Assert.AreEqual(IsSendOnStarImmediate, _waitH2NStart.WaitOne(500));

                // close h2n manually and wait some time for ActivityMonitor::OnHand2NoteStop 
                // Thread.Sleep(10000);
            }

            // ActivityMonitor is IDisposable
            monitor.Dispose();
            monitor.Dispose();
        }
    }

    [TestClass]
    public class H2NApiUtilsTest
    {
        public H2NApiUtilsTest()
        {
            Hand2Note.DLLPath = FileSysHelpers.Hand2NoteApiDLLPath;
            // free dll before test cases
            Hand2Note.FreeLibrary();
        }

        [TestMethod]
        public void TestTableName()
        {
            // hand2note only accept handhistories and dynamic data with XXXXnnnnnnn table name format, where XXXX is room prefix, nnnnnnn - some hash from original table name
            // to convert original table name to format supported by h2n use [Utils.MakeTableName]

            var s1 = Hand2Note.GetRoomDefiningTableName(Rooms.FishPokers, "some table name");
            Assert.IsTrue( Regex.IsMatch(s1, @"^FSHP[0-9]+$") );

            var s2 = Hand2Note.GetRoomDefiningTableName(Rooms.FishPokers, "some table name");
            // same original table name - same output
            Assert.IsTrue(s1 == s2);

            var s3 = Hand2Note.GetRoomDefiningTableName(Rooms.FishPokers, "table name");
            Assert.IsFalse(s1 == s3);


            var s4 = Hand2Note.GetRoomDefiningTableName(Rooms.PokerStars, "table name");
            // same original table name but different prefix
            Assert.IsFalse(s3 == s4);

            // prefix depends on room
            Assert.IsTrue( Regex.IsMatch(s4, @"^PS[0-9]+$") );
        }
    }


    [TestClass]
    public class H2NApiSendCompletedHandHistoryTest
    {
        public H2NApiSendCompletedHandHistoryTest()
        {
            Hand2Note.DLLPath = FileSysHelpers.Hand2NoteApiDLLPath;
            // free dll before test cases
            Hand2Note.FreeLibrary();
        }

        // completed handhistory in pokerstars format
        static readonly string PokerFishFormattedHandHistory = @"PokerStars Hand #2416948123: Hold'em No Limit ($0.25/$0.50 USD) - 2019/01/21 13:44:57 ET
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
";

        [TestMethod]
        public void TestMessageSend()
        {
            // we dont need to check is h2n running or not, messages are written into internal ipc circle buffer
            // if h2n is running its gonna grab em from ipc queue and process

            // --------------------------------------------------------------------------------------------------------------
            // need to convert table name 
            
            // get original table name
            var tableName = Regex.Match(PokerFishFormattedHandHistory, @"Table '(.+)'").Groups[1].Value;

            Assert.AreEqual(@"大安上王33", tableName);

            // make table name required by h2n
            var h2nTableName = Hand2Note.GetRoomDefiningTableName(Rooms.FishPokers, tableName);
            Assert.IsTrue(Regex.IsMatch(h2nTableName, @"^FSHP[0-9]+$"));

            // replace
            var HandHistory =  PokerFishFormattedHandHistory.Replace(tableName, h2nTableName);


            // --------------------------------------------------------------------------------------------------------------
            // create hand history message for hand2note
            var msg = new HandHistoryMessage(); // or new HandHistoryMessage(Rooms.PokerFish, 2416948123, HandHistoryFormats.PokerStars, HandHistory)
            
            // hand history format is pokerstars
            msg.Format = HandHistoryFormats.Stars;

            // next params required by h2n for quick search / reject dups w/o parsing handhistory
            // room is pokerfishS
            msg.Room = Rooms.FishPokers;
            // game id / hand id
            msg.GameNumber = long.Parse(Regex.Match(PokerFishFormattedHandHistory, @"Hand #([0-9]+):").Groups[1].Value);
            Assert.AreEqual(2416948123, msg.GameNumber);
            // lets randomize it for testing
            int newGameId = (new Random()).Next(int.MaxValue / 2, int.MaxValue);
            HandHistory = HandHistory.Replace(msg.GameNumber.ToString(), newGameId.ToString());
            msg.GameNumber = newGameId;

            // IsZoom == false by default
            Assert.IsFalse(msg.IsZoom);

            // handhistory in msg.Format (pokerstars)
            msg.HandHistory = HandHistory;

            // original handhistory is internal room format, for example xml for connective
            // for pokerfish there is no original format
            Assert.AreEqual("", msg.OriginalHandHistory);


            // send message, check h2n smart inspect (.sil) log for
            // 'Integrated Hh #XXXX = "PokerStars Hand #XXXX ...' message
            Hand2Note.Send(msg);
        }
    }


    [TestClass]
    public class H2NApiSendHandDynamic
    {
        public H2NApiSendHandDynamic()
        {
            Hand2Note.DLLPath = FileSysHelpers.Hand2NoteApiDLLPath;
            // free dll before test cases
            Hand2Note.FreeLibrary();
        }

        // Dynamic hand template

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


        [TestMethod]
        public void TestMessageSend()
        {
            // we dont need to check is h2n running or not, messages are written into internal ipc circle buffer
            // if h2n is running its gonna grab em from ipc queue and process

            var msg = new HandStartMessage();
            msg.Ante = 0.25;
            msg.SmallBlind = 0.25;
            msg.BigBlind = 0.5;
            msg.Currency = Currencies.Yuan;
            // real gameId is 2416948123, but we'll use random for testing
            msg.GameNumber = (new Random()).Next(int.MaxValue / 2, int.MaxValue);
            msg.Room = Rooms.FishPokers;
            msg.TableSize = 9;
            // need table name in h2n format, XXXXnnnn
            msg.TableName = Hand2Note.GetRoomDefiningTableName(Rooms.FishPokers, @"大安上王33");
            Assert.IsTrue(Regex.IsMatch(msg.TableName, @"^FSHP[0-9]+$"));

            // check default values...
            Assert.AreEqual(0, msg.Straddle); // always set straddle if there is any
            Assert.IsFalse(msg.IsCap);
            Assert.IsFalse(msg.IsLimit);
            Assert.IsFalse(msg.IsOmaha);
            Assert.IsFalse(msg.IsPotLimit);
            Assert.IsFalse(msg.IsTourney);
            Assert.IsFalse(msg.IsZoom);

            var hash = new MurmurHash();

            // there are 6 seats in template
            // -- seat 1 -------------------------------------------------------
            var s1 = new PlayerSeatInfo();
            s1.Nickname = @"无能为力";
            // player id used in china rooms, fill it with hash if you cant get real playerId
            s1.PlayerShowId = hash.Hash(s1.Nickname).ToString();
            // h2n seat indexes are in [0..MaxPlayers-1]
            s1.SeatIndex = 0; // 1
            s1.InitialStackSize = 109.54;
            // seat 1 posted big blind
            s1.IsPostedBigBlind = true;
            // everything else is false by default
            Assert.IsFalse(s1.IsDealer || s1.IsHero || s1.IsPostedSmallBlind ||
                s1.IsPostedStraddle || s1.IsPostedSmallBlindOutOfQueue || s1.IsPostedBigBlindOutOfQueue ||
                s1.IsSittingOut);
            // no poket cards
            Assert.IsTrue(string.IsNullOrEmpty(s1.PoketCards));
            msg.Seats.Add(s1);

            // -- seat 3 -------------------------------------------------------
            var s3 = new PlayerSeatInfo();
            s3.Nickname = @"张琳";
            s3.PlayerShowId = hash.Hash(s3.Nickname).ToString();
            s3.SeatIndex = 2; // 3
            s3.InitialStackSize = 168.45;
            Assert.IsFalse(s3.IsDealer || s3.IsHero || s3.IsPostedSmallBlind || s3.IsPostedBigBlind ||
                s3.IsPostedStraddle || s3.IsPostedSmallBlindOutOfQueue || s3.IsPostedBigBlindOutOfQueue ||
                s3.IsSittingOut);
            Assert.IsTrue(string.IsNullOrEmpty(s3.PoketCards));
            msg.Seats.Add(s3);

            // -- seat 4 -------------------------------------------------------
            var s4 = new PlayerSeatInfo();
            s4.Nickname = @"天天大水上";
            s4.PlayerShowId = hash.Hash(s4.Nickname).ToString();
            s4.SeatIndex = 3; // 4
            s4.InitialStackSize = 59.26;
            Assert.IsFalse(s4.IsDealer || s4.IsHero || s4.IsPostedSmallBlind || s4.IsPostedBigBlind ||
                s4.IsPostedStraddle || s4.IsPostedSmallBlindOutOfQueue || s4.IsPostedBigBlindOutOfQueue ||
                s4.IsSittingOut);
            Assert.IsTrue(string.IsNullOrEmpty(s4.PoketCards));
            msg.Seats.Add(s4);


            // -- seat 5 -------------------------------------------------------
            var s5 = new PlayerSeatInfo();
            s5.Nickname = @"安排！";
            s5.PlayerShowId = hash.Hash(s5.Nickname).ToString();
            s5.SeatIndex = 4; // 5
            s5.InitialStackSize = 48.14;
            Assert.IsFalse(s5.IsDealer || s5.IsHero || s5.IsPostedSmallBlind || s5.IsPostedBigBlind ||
                s5.IsPostedStraddle || s5.IsPostedSmallBlindOutOfQueue || s5.IsPostedBigBlindOutOfQueue ||
                s5.IsSittingOut);
            Assert.IsTrue(string.IsNullOrEmpty(s5.PoketCards));
            msg.Seats.Add(s5);

            // -- seat 7 -------------------------------------------------------
            var s7 = new PlayerSeatInfo();
            s7.Nickname = @"木樽";
            s7.PlayerShowId = hash.Hash(s7.Nickname).ToString();
            s7.SeatIndex = 6; // 7
            s7.InitialStackSize = 247.19;
            // seat 7 is dealer
            s7.IsDealer = true;
            Assert.IsFalse(s7.IsHero || s7.IsPostedSmallBlind || s7.IsPostedBigBlind ||
                s7.IsPostedStraddle || s7.IsPostedSmallBlindOutOfQueue || s7.IsPostedBigBlindOutOfQueue ||
                s7.IsSittingOut);
            Assert.IsTrue(string.IsNullOrEmpty(s7.PoketCards));
            msg.Seats.Add(s7);

            // -- seat 8 -------------------------------------------------------
            var s8 = new PlayerSeatInfo();
            s8.Nickname = @"德州小丑王";
            s8.PlayerShowId = hash.Hash(s8.Nickname).ToString();
            s8.SeatIndex = 7; // 8
            s8.InitialStackSize = 53.82;
            // seat 8 posted small blind
            s8.IsPostedSmallBlind = true;
            // lets say its hero
            s8.IsHero = true; 
            Assert.IsFalse(s8.IsDealer || s8.IsPostedBigBlind ||
                s8.IsPostedStraddle || s8.IsPostedSmallBlindOutOfQueue || s8.IsPostedBigBlindOutOfQueue ||
                s8.IsSittingOut);
            // is hero flag can be used with or without poket cards, h2n uses IsHero to arrange huds
            Assert.IsTrue(string.IsNullOrEmpty(s8.PoketCards));
            msg.Seats.Add(s8);

            Assert.AreEqual(6, msg.Seats.Count);

            // -------------------------------------------------------------------
            // ok we need to assign 'hand start' message to some window via HWND
            // open notepad, find notepad window handle with spy++ and setup msg.TableWindowHwnd
            msg.TableWindowHwnd = 0x00B1235C;
            if (WinApiHelper.IsWindow((IntPtr)msg.TableWindowHwnd) == 0)
            {
                Console.WriteLine("HandStart Message requires valid window handle");
            }
            Assert.AreNotEqual(0, WinApiHelper.IsWindow((IntPtr)msg.TableWindowHwnd));

            // send message, h2n huds should appear on notepad window
            Hand2Note.Send(msg);

            // -- actions ------------------------------------------------------------------

            int DELAY = 500;

            // POT = 2.25
            // 张琳: raises $0.50 to $1
            var a1_msg = new HandActionMessage();
            a1_msg.GameNumber = msg.GameNumber;
            a1_msg.ActionType = Actions.Raise;
            a1_msg.Amount = 0.5;
            a1_msg.SeatIndex = 2;
            Hand2Note.Send( a1_msg);

            Thread.Sleep(DELAY);

            // POT = 2.75
            // 天天大水上: folds
            var a2_msg = new HandActionMessage();
            a2_msg.GameNumber = msg.GameNumber;
            a2_msg.ActionType = Actions.Fold;
            Assert.AreEqual(0, a2_msg.Amount);
            a2_msg.SeatIndex = 3;
            Hand2Note.Send(a2_msg);

            Thread.Sleep(DELAY);

            // POT = 2.75
            //安排！: folds
            var a3_msg = new HandActionMessage();
            a3_msg.GameNumber = msg.GameNumber;
            a3_msg.ActionType = Actions.Fold;
            Assert.AreEqual(0, a3_msg.Amount);
            a3_msg.SeatIndex = 4;
            Hand2Note.Send(a3_msg);

            Thread.Sleep(DELAY);

            // POT = 2.75
            //木樽: folds
            var a4_msg = new HandActionMessage();
            a4_msg.GameNumber = msg.GameNumber;
            a4_msg.ActionType = Actions.Fold;
            Assert.AreEqual(0, a4_msg.Amount);
            a4_msg.SeatIndex = 6;
            Hand2Note.Send(a4_msg);

            Thread.Sleep(DELAY);

            // POT = 2.75
            //德州小丑王: calls $0.75
            var a5_msg = new HandActionMessage();
            a5_msg.GameNumber = msg.GameNumber;
            a5_msg.ActionType = Actions.Call;
            a5_msg.Amount = 0.75;
            a5_msg.SeatIndex = 7;
            Hand2Note.Send(a5_msg);

            Thread.Sleep(DELAY);

            // POT = 3.50
            //无能为力: folds
            var a6_msg = new HandActionMessage();
            a6_msg.GameNumber = msg.GameNumber;
            a6_msg.ActionType = Actions.Fold;
            Assert.AreEqual(0, a6_msg.Amount);
            a6_msg.SeatIndex = 0;
            Hand2Note.Send(a6_msg);


            Thread.Sleep(DELAY);

            // POT = 3.50
            //*** FLOP *** [5h 8s 7s]
            var flop_msg = new HandDealMessage();
            flop_msg.GameNumber = msg.GameNumber;
            flop_msg.Board = "5h8s7s";
            flop_msg.Street = Streets.Flop;
            flop_msg.Pot = 3.50;
            Hand2Note.Send(flop_msg);
            //flop_msg.Send();


            Thread.Sleep(DELAY);

            // POT = 3.50
            //德州小丑王: checks
            var a7_msg = new HandActionMessage();
            a7_msg.GameNumber = msg.GameNumber;
            a7_msg.ActionType = Actions.Check;
            Assert.AreEqual(0, a7_msg.Amount);
            a7_msg.SeatIndex = 7;
            Hand2Note.Send(a7_msg);

            Thread.Sleep(DELAY);

            // POT = 3.50
            //张琳: bets $2.66
            var a8_msg = new HandActionMessage();
            a8_msg.GameNumber = msg.GameNumber;
            a8_msg.ActionType = Actions.Bet;
            a8_msg.Amount = 2.66;
            a8_msg.SeatIndex = 2;
            Hand2Note.Send(a8_msg);

            Thread.Sleep(DELAY);

            // POT = 6.16
            //德州小丑王: calls $2.66
            var a9_msg = new HandActionMessage();
            a9_msg.GameNumber = msg.GameNumber;
            a9_msg.ActionType = Actions.Call;
            a9_msg.Amount = 2.66;
            a9_msg.SeatIndex = 7;
            Hand2Note.Send(a9_msg);

            Thread.Sleep(DELAY);

            // POT = 8.82
            //*** TURN *** [5h 8s 7s] [Ts]
            var turn_msg = new HandDealMessage();
            turn_msg.GameNumber = msg.GameNumber;
            turn_msg.Board = "5h8s7sTs";
            turn_msg.Street = Streets.Turn;
            turn_msg.Pot = 8.82;
            Hand2Note.Send(turn_msg);

            Thread.Sleep(DELAY);

            // POT = 8.82
            //德州小丑王: checks
            var a10_msg = new HandActionMessage();
            a10_msg.GameNumber = msg.GameNumber;
            a10_msg.ActionType = Actions.Check;
            Assert.AreEqual(0, a10_msg.Amount);
            a10_msg.SeatIndex = 7;
            Hand2Note.Send(a10_msg);

            Thread.Sleep(DELAY);

            // POT = 8.82
            // 张琳: bets $6.21
            var a11_msg = new HandActionMessage();
            a11_msg.GameNumber = msg.GameNumber;
            a11_msg.ActionType = Actions.Bet;
            a11_msg.Amount = 6.21;
            a11_msg.SeatIndex = 2;
            Hand2Note.Send(a11_msg);

            Thread.Sleep(DELAY);

            // POT = 15.03
            // 德州小丑王: folds
            var a12_msg = new HandActionMessage();
            a12_msg.GameNumber = msg.GameNumber;
            a12_msg.ActionType = Actions.Fold;
            Assert.AreEqual(0, a12_msg.Amount);
            a12_msg.SeatIndex = 7;
            Hand2Note.Send(a12_msg);
        }
    }

    [TestClass]
    public class H2NApiSendCommand
    {
        public H2NApiSendCommand()
        {
            Hand2Note.DLLPath = FileSysHelpers.Hand2NoteApiDLLPath;
            // free dll before test cases
            Hand2Note.FreeLibrary();
        }


        [TestMethod]
        public void TestCloseHudCommand()
        {
            Hand2Note.Send(123, Rooms.FishPokers, Commands.CloseHud);
        }

        [TestMethod]
        public void TestSendCloseHudCommand()
        {
            Hand2Note.SendCloseHud(123);
        }

        [TestMethod]
        public void TestTableNeedReopenCommand()
        {
            Hand2Note.Send(123, Rooms.FishPokers, Commands.TableNeedsReopen);
        }
    }

    public class MurmurHash
    {
        public UInt32 Hash(string str)
        {
            return Hash(Encoding.UTF8.GetBytes(str), 0xc58f1a7b);
        }
        public UInt32 Hash(Byte[] data)
        {
            return Hash(data, 0xc58f1a7b);
        }
        const UInt32 m = 0x5bd1e995;
        const Int32 r = 24;

        public UInt32 Hash(Byte[] data, UInt32 seed)
        {
            Int32 length = data.Length;
            if (length == 0)
                return 0;
            UInt32 h = seed ^ (UInt32)length;
            Int32 currentIndex = 0;
            while (length >= 4)
            {
                UInt32 k = BitConverter.ToUInt32(data, currentIndex);
                k *= m;
                k ^= k >> r;
                k *= m;

                h *= m;
                h ^= k;
                currentIndex += 4;
                length -= 4;
            }
            switch (length)
            {
                case 3:
                    h ^= BitConverter.ToUInt16(data, currentIndex);
                    h ^= (UInt32)data[currentIndex + 2] << 16;
                    h *= m;
                    break;
                case 2:
                    h ^= BitConverter.ToUInt16(data, currentIndex);
                    h *= m;
                    break;
                case 1:
                    h ^= data[currentIndex];
                    h *= m;
                    break;
                default:
                    break;
            }

            h ^= h >> 13;
            h *= m;
            h ^= h >> 15;

            return h;
        }
    }

}
