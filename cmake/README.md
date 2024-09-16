## What's 'macbundle.cmake' for?

This contains cmake instructions to build MacOS GUI `.app` bundle. It embeds the icon to the app bundle. It also copies all static assets into the bundle at build time.

If you installed SFML as "Frameworks", you will need to tell XCode to copy them into the bundle before Building. Follow these steps: 

- From top toolbar inside Xcode, click **Product** > **Add New Build Phase** > choose **Copy Files**. See official [Apple guide](https://developer.apple.com/documentation/xcode/customizing-the-build-phases-of-a-target)
- Click the `+` (add button). A new window will pop up.
- Now select all SFML frameworks listed: `sfml-graphics.framework` `sflml-window.framework` etc. You should also add `Foundation` framework.
- Finally, you will need to find `freetype.framework`. Click **Other Locations**, find it inside `/Library/Frameworks`.

Now you can build your project. Choose **Product** from top toolbar, click **Build**.
