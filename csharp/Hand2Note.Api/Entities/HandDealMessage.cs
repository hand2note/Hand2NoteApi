using System;
using System.Collections.Generic;
using System.Text;

namespace Hand2Note.Api
{
    /// <summary>
    /// Send dynamic poker street message to Hand2Note, used with <see cref="HandActionMessage"/> to maintain dynamic deal state after <see cref="HandStartMessage"/>
    /// </summary>
    public class HandDealMessage
    {
        /// <summary>
        /// Game/Hand number
        /// </summary>
        public long GameNumber { set; get; } = 0;
        /// <summary>
        /// Street type. Flop/Turn/River
        /// </summary>
        public Streets Street { set; get; } = Streets.Flop;
        /// <summary>
        /// Board cards string in common format '5cQdTh'. 3 cards for flop, 4 for turn, 5 for river
        /// </summary>
        public string Board { set; get; } = "";
        /// <summary>
        /// Total pot value on street. Is not required for hands without rake.
        /// </summary>
        public double Pot { set; get; } = 0;

        public HandDealMessage() { }
        public HandDealMessage(long gameNumber, Streets type, string board, double pot = 0)
        {
            GameNumber = gameNumber;
            Street = type;
            Board = board;
            Pot = pot;
        }
    }

}
