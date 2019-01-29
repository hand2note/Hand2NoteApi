using System;
using System.Collections.Generic;
using System.Text;

namespace Hand2Note.Api
{
    /// <summary>
    /// Send message with completed (static) hand history text to Hand2Note. 
    /// </summary>
    public class HandHistoryMessage
    {
        /// <summary>
        /// Target room
        /// </summary>
        public Rooms Room { set; get; }
        /// <summary>
        /// Game/Hand Index
        /// </summary>
        public long GameNumber { set; get; } = 0;
        /// <summary>
        /// Is fastfold, zoom, boost table
        /// </summary>
        public bool IsZoom { set; get; } = false;
        /// <summary>
        /// <c>HandHistory</c> text format, PS/888/WPN 
        /// </summary>
        public HandHistoryFormats Format { set; get; } = HandHistoryFormats.Original;
        /// <summary>
        /// Completed hand history
        /// </summary>
        public string HandHistory { set; get; } = "";

        /// <summary>
        /// Original hand history text if there is any
        /// </summary>
        public string OriginalHandHistory { set; get; } = "";


        public HandHistoryMessage() { }
        public HandHistoryMessage(Rooms room, long gameNumber, HandHistoryFormats format, string formattedHandHistory)
        {
            Room = room; GameNumber = gameNumber; Format = format; HandHistory = formattedHandHistory;
        }

    }
}
