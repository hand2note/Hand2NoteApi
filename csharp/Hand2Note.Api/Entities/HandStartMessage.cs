using System;
using System.Collections.Generic;
using System.Text;

namespace Hand2Note.Api
{
    /// <summary>
    /// Send dynamic hand start message to Hand2Note
    /// </summary>
    /// <seealso cref="HandActionMessage"/>
    /// <seealso cref="HandDealMessage"/>
    public class HandStartMessage
    {
        /// <summary>
        /// Target room
        /// </summary>
        public Rooms Room { set; get; } = Rooms.PokerStars;
        /// <summary>
        /// Game/Hand number
        /// </summary>
        public long GameNumber { set; get; } = 0;
        /// <summary>
        /// Table name in supported format, see <see cref="Hand2Note.GetRoomDefiningTableName(Rooms, string)"/>
        /// </summary>
        public string TableName { set; get; } = "";
        /// <summary>
        /// Table window handle (HWND) ( where to attach huds )
        /// </summary>
        public int TableWindowHwnd { set; get; } = 0;
        /// <summary>
        /// Table max players. 2-max, 6-max, 9-max etc
        /// </summary>
        public int TableSize { set; get; } = 10;
        /// <summary>
        /// Is it tournament hand 
        /// </summary>
        public bool IsTourney { set; get; } = false;
        /// <summary>
        /// Is it omaha hand
        /// </summary>
        public bool IsOmaha { set; get; } = false;
        /// <summary>
        /// Is it fixed limit game
        /// </summary>
        public bool IsLimit { set; get; } = false;
        /// <summary>
        /// Is it zoom/fastfold/boost hand
        /// </summary>
        public bool IsZoom { set; get; } = false;
        /// <summary>
        /// Is there a cap 
        /// </summary>
        public bool IsCap { set; get; } = false;
        /// <summary>
        /// Is it pot limit game
        /// </summary>
        public bool IsPotLimit { set; get; } = false;
        /// <summary>
        /// Is it short deck holdem game
        /// </summary>
        public bool IsShortDeck { set; get; } = false;
        /// <summary>
        /// Is it omaha5 game
        /// </summary>
        public bool IsOmahaFive { set; get; } = false;
        /// <summary>
        /// Room currency
        /// </summary>
        public Currencies Currency { set; get; } = Currencies.Dollar;
        /// <summary>
        /// Small blind amount
        /// </summary>
        public double SmallBlind { set; get; } = 0;
        /// <summary>
        /// Big blind amount
        /// </summary>
        public double BigBlind { set; get; } = 0;
        /// <summary>
        /// Ante amount
        /// </summary>
        public double Ante { set; get; } = 0;
        /// <summary>
        /// Straddle amount
        /// </summary>
        public double Straddle { set; get; } = 0;

        /// <summary>
        /// List of table seats.
        /// </summary>
        public List<PlayerSeatInfo> Seats { set; get; } = new List<PlayerSeatInfo>();

        public HandStartMessage() { }
        public HandStartMessage(Rooms room, long gameNumber = 0)
        {
            GameNumber = gameNumber; Room = room;
        }

        
    }
}
