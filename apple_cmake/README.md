## What's this for

This contains cmake instructions to build MacOS GUI `.app` bundle. It embeds the icon to the app bundle.  It also copies all assets/Resources nicely into the bundle at build time. 

But still cannot copy the Frameworks (libs) into the bundle. This is where Xcode manual copy comes in. YOu will need to manually embed the frameworks using Xcode to the bundle before building for Release. Just a few clicks in Xcode, Add New Build Step > Browse all SFML libs listed. Also you will need to find `freetype.framework` file inside /Library/Frameworks and embed it too.
