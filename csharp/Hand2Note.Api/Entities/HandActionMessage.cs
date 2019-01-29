using System;
using System.Collections.Generic;
using System.Text;

namespace Hand2Note.Api
{

    /// <summary>
    /// Send dynamic poker action message to Hand2Note, used with <see cref="HandDealMessage"/> to maintain dynamic deal state after <see cref="HandStartMessage"/>
    /// </summary>
    public class HandActionMessage
    {
        /// <summary>
        /// Hand game number
        /// </summary>
        public long GameNumber { set; get; } = 0;
        /// <summary>
        /// Action seat index in range [0..MaxPlayers-1]
        /// </summary>
        public int SeatIndex { set; get; } = -1;
        /// <summary>
        /// Action type. Fold/Check/Raise etc
        /// </summary>
        public Actions ActionType { set; get; } = Actions.Fold;
        /// <summary>
        /// Call/Bet/Raise amount. If the ActionType is raise then Amount should contain the value "raised to", i.e. the current bet amount of the player .
        /// </summary>
        public double Amount { set; get; } = 0;
        
       
        public HandActionMessage() { }
        public HandActionMessage(long gameNumber, int seatIndex, Actions actionType, double amount = 0)
        {
            GameNumber = gameNumber;
            SeatIndex = seatIndex;
            ActionType = actionType;
            Amount = amount;
        }
    }
}
