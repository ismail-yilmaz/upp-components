MessageCtrl package for U++
-----------------------------

This class implements a messages manager. 
Messages are simple message boxes similar to prompts in that they can allow the same basic user
actions. However, message boxes are not meant as a replacement for the traditional U++ dialogs.
The main difference between the messages and the prompts is that the message boxes are implemented
as frames instead of dialogs, and are meant to be less intrusive, and non-blocking.

There are several types of messages:

- Information: Should be used to display common information.
- Warning:     Should be used to report non-critical issue, to inform not everything is all right.
- Success:     Should be used to display successful operations. Essentially this is an information notification.
- Question:    Should be used to ask for some interaction.
- Error:       Should be used to display critical application errors
- Custom:      If the predefined notification types don't suit your needs, you can create one.

Message boxes use QTF texts. This allows for embedding hyperlinks in messages. 
It also means that in some cases the text messages should be escaped, using DeQtf() function.

History:
--------------------
2018-04-07: UseCross() method added. It is now possible to use a
            small image button with cross instead of OK button.
            Informative message boxes use crosses by default.

2018-03-03: Name clash on Windows fixed.
            MessageBox::Type::ERROR  -> MessageBox::Type::FAILURE
            Message dsiplay order changed. New messages will be inserted
            as the the topmost/bottom-most frame (depending on orientation)
			"Selective" clearing added. 

2018-03-01: Information and custom message boxes can now have timeouts.

2018-02-21: Widget renames as MessageCtrl. (Final)

2018-02-20: It is now possible to add mesasges as both top and bottom frames.
            It is now possible to add single message box without using the manager.
            Message framework further refactored.


2018-02-19: Name change: Notification -> Message.
            Code refactored. (Thanks Klugier!)

2018-02-18: Initial public release
