
# MessageCtrl package for Ultimate++
------------------------------------

This package implements a messages manager widget. Messages are simple message boxes similar to prompts in that they can allow the same basic user actions. However, message boxes are not meant as a replacement for the traditional U++ dialogs. The main difference between the messages and the prompts is that the message boxes are implemented as frames instead of dialogs, and are meant to be less intrusive, and non-blocking.

There are several types of messages:

-   Information: Should be used to display common information.
-   Warning: Should be used to report non-critical issue, to inform not everything is all right.
-   OK: Should be used to display successful operations. Essentially this is an information notification.
-   Question: Should be used to ask for some interaction.
-   Error: Should be used to display critical application errors
-   Custom: If the predefined notification types don't suit your needs, you can create one.

Message boxes use QTF texts. This allows for embedding hyperlinks in messages. It also means that in some cases the text messages should be escaped, using DeQtf() function.

------------------------------------

![Screenshot](https://i.imgur.com/dq9pBUz.png)