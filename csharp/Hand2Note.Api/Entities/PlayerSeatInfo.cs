namespace Hand2Note.Api
{
    /// <summary>
    /// Seat data class, used in <c>HandStartMessage</c>. Contains nickname, index, stack, blinds etc
    /// </summary>
    public class PlayerSeatInfo
    {
        /// <summary>
        /// Seat index in range [0..MaxPlayers-1]. SeatIndex == 0 if the seat is at 18 o'clock.
        /// </summary>
        public int SeatIndex { set; get; } = -1;
        /// <summary>
        /// Player nickname. On Chinese poker rooms this should be the visible nickname of the player, i.e. those one usually in Chinese letters.
        /// </summary>
        public string Nickname { set; get; } = "";
        /// <summary>
        /// Player ID used in china rooms, set it if possible or use hash code from nickname
        /// </summary>
        public string PlayerShowId { set; get; }
        /// <summary>
        /// Starting stack size, before blinds and ante
        /// </summary>
        public double InitialStackSize { set; get; } = 0;
        /// <summary>
        /// Poket cards string in common format : '6sKcTdAh'. Can be empty even if <c>IsHero</c> set to true
        /// </summary>
        public string PoketCards { set; get; } = "";
        /// <summary>
        /// Player is a dealer/button
        /// </summary>
        public bool IsDealer { set; get; } = false;
        /// <summary>
        /// Seat posted small blind
        /// </summary>
        public bool IsPostedSmallBlind { set; get; } = false;
        /// <summary>
        /// Seat posted big blind
        /// </summary>
        public bool IsPostedBigBlind { set; get; } = false;
        /// <summary>
        /// Seat posted small blind out of queue, usually dead blind
        /// </summary>
        public bool IsPostedSmallBlindOutOfQueue { set; get; } = false;
        /// <summary>
        /// Seat posted big blind out of queue, entry bet
        /// </summary>
        public bool IsPostedBigBlindOutOfQueue { set; get; } = false;
        /// <summary>
        /// Seat posted straddle
        /// </summary>
        public bool IsPostedStraddle { set; get; } = false;
        /// <summary>
        /// Seat is hero, important flag used to arrange huds around preferred seat.
        /// </summary>
        public bool IsHero { set; get; } = false;
        /// <summary>
        /// Seat is sitting out
        /// </summary>
        public bool IsSittingOut { set; get; } = false;

        public PlayerSeatInfo() { }
        public PlayerSeatInfo(string nickname, string playerShowId, int seatIndex, double initialStackSize, bool isHero = false)
        {
            Nickname = nickname;
            PlayerShowId = playerShowId;
            SeatIndex = seatIndex;
            InitialStackSize = initialStackSize;
            IsHero = isHero;
        }

    }
}