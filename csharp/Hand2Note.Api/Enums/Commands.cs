namespace Hand2Note.Api
{
    /// <summary>
    /// Commands to Hand2Note
    /// </summary>
    public enum Commands
    {
        /// <summary>
        /// Hand2Note closes the HUD for the specified TableHWnd.
        /// Use this command when the user closes the table but keeps its window open (like on Asian mobile poker rooms) to shut down the HUD in Hand2Note.
        ///
        /// You don't need to send this message when the table's window was actually closed
        /// because Hand2Note is able to detect the closed window and shut down the HUD itself.
        /// </summary>
        CloseHud = 0 ,

        /// <summary>
        /// Hand2Note shows message in the table's HUD: "Please, reopen the table."
        /// </summary>
        TableNeedsReopen = 3,

        /// <summary>
        /// Hand2Note shows message in the table's HUD: "Please, restart emulator."
        /// </summary>
        EmulatorNeedRestart = 4,
    }
}