using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace Hand2Note.Api
{
    /// <summary>
    /// Hand2Note activity monitor. Contains methods for checking if Hand2Note is running, events for Hand2Note start and/or close.
    /// </summary>
    public class ActivityMonitor : IDisposable
    {
        /// <summary>
        /// Checks if Hand2Note is running
        /// </summary>
        public static bool IsHand2NoteRunning => Hand2Note.IsHand2NoteRunning();

        /// <summary>
        /// Time interval between IsHand2NoteRunning calls, used to fire <c>OnHand2NoteStart</c> and <c>OnHand2NoteClose</c> events
        /// </summary>
        public int PollDelayMs { get; set; } = 300;

        private Task _pollTask = null;
        private CancellationTokenSource _cancellationTokenSource;
        private bool _immediateOnStart = true;
        private bool _isRunning;


        /// <summary>
        /// Fired on when Hand2Note client is started
        /// </summary>
        /// <remarks>
        /// Fired immediately if ActivityMonitor was created with immediateOnStart == true
        /// </remarks>
        public event Action Hand2NoteStarted;
        /// <summary>
        /// Fired when Hand2Note client becomes closed
        /// </summary>
        public event Action Hand2NoteClosed;


        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="immediateOnStart">if true and Hand2Note running, OnHand2NoteStart fired immediately</param>
        public ActivityMonitor(bool immediateOnStart = true)
        {
            _immediateOnStart = immediateOnStart;
            _cancellationTokenSource = new CancellationTokenSource();
            _pollTask = Task.Run(() => Loop(_cancellationTokenSource.Token));
        }

        /// <summary>
        /// ActivityMonitor implements IDisposable
        /// </summary>
        public void Dispose()
        {
            if (_pollTask != null)
            {
                _cancellationTokenSource.Cancel();
                _pollTask.Wait();
                _pollTask = null;
            }
        }

        void Loop(CancellationToken cancellationToken)
        {
            _isRunning = IsHand2NoteRunning;
            if (_immediateOnStart && _isRunning)
            {
                Hand2NoteStarted?.Invoke();
            }

            var WaitHandles = new[] { cancellationToken.WaitHandle };
            while (WaitHandle.WaitAny(WaitHandles, PollDelayMs) == WaitHandle.WaitTimeout)
            {
                var currState = IsHand2NoteRunning;
                if (currState == _isRunning)
                    continue;
                _isRunning = currState;
                if (_isRunning)
                    Hand2NoteStarted?.Invoke();
                else
                    Hand2NoteClosed?.Invoke();
            }
        }

    }
}
