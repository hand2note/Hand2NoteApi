using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace Hand2Note.Api
{

    public static class Hand2Note
    {
        /// <summary>
        /// Default path to dll readonly constant. Corresponding h2napi.dll are in the current x86/x64 subdirectory
        /// </summary>
        private static string DefaultDllPath => $"{(Environment.Is64BitOperatingSystem? "x64" : "x86")}\\h2napi.dll";

        /// <summary>
        /// Path to h2napi.dll
        /// </summary>
        public static string DLLPath { set; get; } = DefaultDllPath;


        public static bool IsHand2NoteRunning()
        {
            lock (_lockObject)
            {
                LazyInitLibrary();
                return _h2nIsRunning() != 0;
            }
        }

        public static void Send(HandHistoryMessage message)
        {
            lock (_lockObject)
            {
                LazyInitLibrary();

                var msg = new h2n_hh_message_struct();
                msg.format = (int)message.Format;
                msg.gameid = (double)message.GameNumber;
                msg.is_zoom = message.IsZoom ? 1 : 0;
                msg.room = (int)message.Room;
                msg.hh_formatted = WinApiHelper.StringToUTF8Pointer(message.HandHistory);
                msg.hh_original = WinApiHelper.StringToUTF8Pointer(message.OriginalHandHistory);
                //todo: handle error codes
                _h2nSendHandHistory(ref msg);
                Marshal.FreeHGlobal(msg.hh_formatted);
                Marshal.FreeHGlobal(msg.hh_original);
            }
        }

        public static void Send(HandStartMessage message)
        {
            lock (_lockObject)
            {
                LazyInitLibrary();

                var msg = new h2n_start_hand_message_struct();
                msg.ante = message.Ante;
                msg.bb = message.BigBlind;
                msg.currency = (int)message.Currency;
                msg.gameid = (double)message.GameNumber;
                msg.is_cap = message.IsCap ? 1 : 0;
                msg.is_limit = message.IsLimit ? 1 : 0;
                msg.is_omaha = message.IsOmaha ? 1 : 0;
                msg.is_potlimit = message.IsPotLimit ? 1 : 0;
                msg.is_shortdeck = message.IsShortDeck ? 1 : 0;
                msg.is_omahafive = message.IsOmahaFive ? 1 : 0;
                msg.is_tourney = message.IsTourney ? 1 : 0;
                msg.is_zoom = message.IsZoom ? 1 : 0;
                msg.max_players = message.TableSize;
                msg.room = (int)message.Room;
                msg.sb = message.SmallBlind;
                msg.straddle = message.Straddle;
                msg.table_hwnd = message.TableWindowHwnd;
                msg.table_name = WinApiHelper.StringToUTF8Pointer(message.TableName);

                var freeList = new List<IntPtr>();
                freeList.Add(msg.table_name);

                msg.seats = new h2n_seat_info_struct[10];
                foreach (var s in message.Seats)
                {
                    var seat = new h2n_seat_info_struct();
                    seat.is_dealer = s.IsDealer ? 1 : 0;
                    seat.is_hero = s.IsHero ? 1 : 0;
                    seat.is_posted_bb = s.IsPostedBigBlind ? 1 : 0;
                    seat.is_posted_bb_outofqueue = s.IsPostedBigBlindOutOfQueue ? 1 : 0;
                    seat.is_posted_sb = s.IsPostedSmallBlind ? 1 : 0;
                    seat.is_posted_sb_outofqueue = s.IsPostedSmallBlindOutOfQueue ? 1 : 0;
                    seat.is_posted_straddle = s.IsPostedStraddle ? 1 : 0;
                    seat.is_sitting_out = s.IsSittingOut ? 1 : 0;
                    seat.nickname = WinApiHelper.StringToUTF8Pointer(s.Nickname);
                    freeList.Add(seat.nickname);
                    seat.player_id = WinApiHelper.StringToUTF8Pointer(s.PlayerShowId);
                    freeList.Add(seat.player_id);
                    seat.pocket_cards = WinApiHelper.StringToUTF8Pointer(s.PoketCards);
                    freeList.Add(seat.pocket_cards);
                    seat.seat_idx = s.SeatIndex;
                    seat.stack = s.InitialStackSize;

                    msg.seats[msg.seats_num++] = seat;
                }

                _h2nSendHandStart(ref msg);
                foreach (var p in freeList)
                    Marshal.FreeHGlobal(p);
            }
        }


        public static void Send(HandActionMessage message)
        {
            lock (_lockObject)
            {
                LazyInitLibrary();

                var msg = new h2n_action_message_struct();
                msg.amount = message.Amount;
                msg.gameid = (double)message.GameNumber;
                msg.is_allin = 0;
                msg.pot = 0;
                msg.seat_idx = message.SeatIndex;
                msg.type = (int)message.ActionType;
                _h2nSendHandAction(ref msg);
            }
        }


        public static void Send(HandDealMessage message)
        {
            lock (_lockObject)
            {
                LazyInitLibrary();

                var msg = new h2n_street_message_struct();
                msg.board = WinApiHelper.StringToUTF8Pointer(message.Board);
                msg.gameid = (double)message.GameNumber;
                msg.pot = message.Pot;
                msg.type = (int)message.Street;
                _h2nSendHandStreet(ref msg);
                Marshal.FreeHGlobal(msg.board);
            }
        }

        /// <summary>
        /// Sends command to Hand2Note to shut down the HUD for tableHWnd
        /// </summary>
        /// <param name="tableHWnd">Table Window handle</param>
        /// <remarks>
        /// Use this command when the user closes the table but keeps its window open (like on Asian mobile poker rooms)
        /// to shut down the HUD in Hand2Note.
        ///
        /// You don't need to send this message when the table's window was actually closed
        /// because Hand2Note is able to detect the closed window and shut down the HUD itself.
        /// </remarks>
        public static void SendCloseHud(int tableHWnd)
        {
            lock (_lockObject)
            {
                LazyInitLibrary();
                _h2nSendCommand(tableHWnd, 0, (int)Commands.CloseHud);
            }
        }
       

        /// <summary>
        /// Sends command to Hand2Note
        /// </summary>
        /// <param name="tableHwnd">Table Window handle</param>
        /// <param name="room">Poker client room</param>
        /// <param name="command">Command</param>
        /// <seealso cref="Commands"/>
        public static void Send(int tableHwnd, Rooms room, Commands command)
        {
            lock (_lockObject)
            {
                LazyInitLibrary();
                _h2nSendCommand(tableHwnd, (int)room, (int)command);
            }
        }

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
        public static string GetRoomDefiningTableName(Rooms room, string originalTableName)
        {
            lock (_lockObject)
            {
                LazyInitLibrary();
                var lpcOriginalTableName = WinApiHelper.StringToUTF8Pointer(originalTableName);
                var lpcTableName = _h2nMakeTableName((int)room, lpcOriginalTableName);
                Marshal.FreeHGlobal(lpcOriginalTableName);
                var ret = WinApiHelper.StringFromUTF8Pointer(lpcTableName);
                _h2nFreeCString(lpcTableName);
                return ret;
            }
        }


        /// <summary>
        /// Call WinApi FreeLibrary for h2napi.dll
        /// </summary>
        /// <remarks>
        /// Is never called, used in test cases, can be used for autoupdate to unlock h2napi.dll file for write
        /// </remarks>
        /// <returns>
        /// true if h2napi.dll was successfully freed
        /// </returns>
        public static bool FreeLibrary()
        {
            lock (_lockObject)
            {
                var ret = (_dllAddress != IntPtr.Zero) && WinApiHelper.FreeLibrary(_dllAddress);
                _dllAddress = IntPtr.Zero;
                return ret;
            }
        }

        private static void LazyInitLibrary()
        {
            if (_dllAddress != IntPtr.Zero)
                return;
            // need absolute path for LoadLibraryW
            var path = Path.GetFullPath(DLLPath);
            if (!File.Exists(DLLPath))
                throw new InvalidOperationException($"Hand2Note Api dll \"{path}\" doesn't exist");
            var addr = WinApiHelper.LoadLibraryW(path);
            if (addr == IntPtr.Zero)
                throw new InvalidOperationException($"Failed to load library \"{path}\": {WinApiHelper.GetLastError()}");

            _h2nIsRunning = (h2n_is_running)LoadDelegate<h2n_is_running>(addr, "h2n_is_running");
            _h2nMakeTableName = (h2n_make_table_name)LoadDelegate<h2n_make_table_name>(addr, "h2n_make_table_name");
            _h2nFreeCString = (h2n_free_cstring)LoadDelegate<h2n_free_cstring>(addr, "h2n_free_cstring");
            _h2nSendHandHistory = (h2n_send_handhistory)LoadDelegate<h2n_send_handhistory>(addr, "h2n_send_handhistory");
            _h2nSendHandStart = (h2n_send_hand_start)LoadDelegate<h2n_send_hand_start>(addr, "h2n_send_hand_start");
            _h2nSendHandAction = (h2n_send_action)LoadDelegate<h2n_send_action>(addr, "h2n_send_action");
            _h2nSendHandStreet = (h2n_send_street)LoadDelegate<h2n_send_street>(addr, "h2n_send_street");
            _h2nSendCommand = (h2n_send_command)LoadDelegate<h2n_send_command>(addr, "h2n_send_command");

            _dllAddress = addr;
        }

        private static Delegate LoadDelegate<T>(IntPtr dllAddress, string functionName)
        {
            var address = WinApiHelper.GetProcAddress(dllAddress, functionName);
            if (address == IntPtr.Zero)
            {
                throw new InvalidOperationException($"Failed to get delegate address for {functionName}");
            }
            return Marshal.GetDelegateForFunctionPointer(address, typeof(T));
        }

        private static IntPtr _dllAddress = IntPtr.Zero;
        static object _lockObject = new object();

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate int h2n_is_running();
        private static h2n_is_running _h2nIsRunning = null;

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate IntPtr h2n_make_table_name(int room, IntPtr UTF8TableName);
        private static h2n_make_table_name _h2nMakeTableName = null;

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate int h2n_send_command(int table_hwnd, int room_id, int cmd);
        private static h2n_send_command _h2nSendCommand = null;

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate IntPtr h2n_free_cstring(IntPtr UTF8TableName);
        private static h2n_free_cstring _h2nFreeCString = null;

        private struct h2n_hh_message_struct
        {
            public int room;
            public int is_zoom;
            public double gameid;
            public int format;
            public IntPtr hh_formatted;
            public IntPtr hh_original;
        }
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate int h2n_send_handhistory([In] ref h2n_hh_message_struct msg);
        private static h2n_send_handhistory _h2nSendHandHistory = null;

        private struct h2n_seat_info_struct
        {
            public int seat_idx;
            public IntPtr nickname;
            public IntPtr player_id;
            public double stack;
            public IntPtr pocket_cards;
            public int is_dealer;
            public int is_posted_sb;
            public int is_posted_bb;
            public int is_posted_sb_outofqueue;
            public int is_posted_bb_outofqueue;
            public int is_posted_straddle;
            public int is_hero;
            public int is_sitting_out;
        }

        private struct h2n_start_hand_message_struct
        {
            public int room;
            public double gameid;
            public IntPtr table_name;
            public int table_hwnd;
            public int max_players;
            public int is_tourney;
            public int is_omaha;
            public int is_limit;
            public int is_zoom;
            public int is_cap;
            public int is_potlimit;
            public int is_shortdeck;
            public int is_omahafive;
            public int currency;
            public double sb;
            public double bb;
            public double ante;
            public double straddle;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 10)]
            public h2n_seat_info_struct[] seats;
            public int seats_num;
        }
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate int h2n_send_hand_start([In] ref h2n_start_hand_message_struct msg);
        private static h2n_send_hand_start _h2nSendHandStart = null;


        private struct h2n_action_message_struct
        {
            public double gameid;
            public int seat_idx;
            public int type;
            public double amount;
            public int is_allin;
            public double pot;
        }
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate int h2n_send_action([In] ref h2n_action_message_struct msg);
        private static h2n_send_action _h2nSendHandAction = null;


        private struct h2n_street_message_struct
        {
            public double gameid;
            public int type;
            public IntPtr board;
            public double pot;
        }
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate int h2n_send_street([In] ref h2n_street_message_struct msg);
        private static h2n_send_street _h2nSendHandStreet = null;

    }
}
