# Hand2Note API

Contains Hand2Note API for third party tools integration.

Hand2Note is an online poker HUD software at https://hand2note.com

Currently available on C# and C++.

**Installation**

For .Net copy _bin/x86/h2napi.dll_ and _bin/x64/h2napi.dll_ into the output directory of your project. Add _bin/Hand2Note.Api.dll_ as a dependency to your project. 


## HUD integration

Use it if you need to show dynamic or static HUD in any window or you need to send a hand history to Hand2Note, process it and save it into the database.

This API is intended for third party converters on poker rooms unsupported by Hand2Note itself. This API should **NOT** be used to violate poker rooms' software restrictions especially on **PokerStars** and measures to maintain the complience will be taken by us.

Users will require PokerMaster HUD subscription in order to use this API with your tool on asian poker rooms. Please, do not try to avoid this subscription requirement.

Minimal working example:
```C#
using Hand2Note.Api;

public void Hand2NoteShowHUDExample(int tableHWnd)
{
    Hand2Note.Send(new HandStartMessage(){
      Room = Rooms.PokerMaster,
      TableWindowHwnd = tableHWnd,
      TableSize = 8,
      Currency = Currencies.Yuan,
      SmallBlind = 5, 
      BigBlind = 10,
      Ante = 2,
      Straddle = 20,
      Seats = ... //Fill List<PlayerSeatInfo>
}
```

General use cases:
```C#
//Notifies Hand2Note that the new hand has been started
Hand2Note.Send(new HandStartMessage() {...});

//Notifies Hand2Note that the street has been changed
Hand2Note.Send(new HandDealMessage(){...});

//Notifies Hand2Note that the action has been done
Hand2Note.Send(new HandActionMessage(){...});

//Sends a "static" hand history to Hand2Note to update players' statistics.
//Please, note that the hand history should contain information about the original poker room it was played on. 
//Hand2Note uses a prefix in the table name to detect the original room.
//Use Hand2Note.GetRoomDefiningTableName method to generate a table name with a correspondent room prefix.
Hand2Note.Send(new HandHistoryMessage(){...});


//Checks if Hand2Note is running
if (Hand2Note.IsHand2NoteRunning()){
...
}

//Use Activity monitor to know when Hand2Note client was started or closed
var activityMonitor = new ActivityMonitor();
activityMonitor.Hand2NoteStarted += Hand2NoteStartedHandler;
activityMonitor.Hand2NoteClosed += Hand2NoteClosedHandler;
```
