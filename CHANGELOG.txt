CHANGELOG: Prefabricator
========================
Version 1.5.2
-------------
* New: Added engine 4.25 support
* New: Project settings has default thumbnail settings which will be used while saving the prefab asset

Version 1.5.1
-------------
* Fix: Improved caching of prefabs

Version 1.5.0
-------------
* New: Prefab serialization is more robust and should properly save all the properties, including the private C++ non-exposed properties
* Fix: Actor references within a prefab are saved and restored correctly.  If there are two actors in a prefab A and B, and if A has a variable which references B,  instantiating this prefab will correctly hold the references to the newly spawned actor B
* Fix: Fixed a bug where existing prefab instances were not reloading to the latest state when the prefab asset was updated
* New: Exposed thumbnail rotation values in the prefab asset.  Double click a prefab to open its properties,  adjust the thumbnail rotation values and then recapture the thumbnail from the asset's context menu
* Fix: Perform proper cleanup when the editor module unloads, removing the custom detail layout customizations on exit

Version 1.4.0
-------------
* Fix: Massive improvements to runtime prefab spawning.   Spawn thousands of prefabs at runtime with minimal overhead
* New: Procedural Platformer Level sample: Build a platformer level by assembling small building blocks and using those to build more complex nested prefabs, eventually building one nested layout prefab with thousands of valid playable layout randomizations.  [Video](https://www.youtube.com/watch?v=RZaYUf_H8fI)
* New: Construction System - Enable your players to build their own worlds by assembling floors, walls, ramps corrdiors.    There's a game sample demonstrating this. [Video](https://www.youtube.com/watch?v=jM0IItlTxjg)
* New: Rewrote the thumbnail renderer, drastically improving the performance.  The thumbs are rendered and cached only while saving the prefab.  This avoid performance issues in the content browser for larger prefabs where they were previously rendered on demand
* New: Prefabs hold soft references to child prefab assets.  This avoids breaking reference issues on child prefabs when their location was changed in the content browser
* Fix: Fixed a crash issue when prefab settings were changed from other editor windows (like Dungeon Architect's theme editor window)
* Fix: Prefabricator loads correctly in custom viewports (like Dungeon Architect's theme editor viewport)
* Fix: Deep nested prefabs sometimes had their mobility incorrectly set to movable (when they were static).  This also fixes the actor hierarchy unlinking issues where the actors were moved out of the prefab hierarchy
* New: Added a new samples submodule under Content/Samples.  All sample content reside in this submodule to avoid increasing the main code repository's size
* New: Prefab Randomizer actor has a delegate to notify when randomization finishes
* Fix: Collision profiles are now saved correctly (Doesn't work in some cases and is still an open issue)
* New: Updated the prefab toolbar icon in the editor window and the prefab actor icon
* New: Moved the documentation to sphinx-docs for cleaner organization and maintainance.  Updated the docs with the new construction system feature.  [Link](https://docs.prefabricator.io)
* New: Disabled Undo / Redo transactions on creating prefabs to improve performance while mass creating prefabs.  This might be restored in the future versions

Version 1.3.1-hf1
-----------------
* New: Prefab Randomizer actor has a delegate to notify when randomization finishes

Version 1.3.0
-------------
* New: Added a project setting to disable custom thumb renderer as it was causing performance issues on larger prefabs.    If it is disabled, another option will be preset to manually set the thumbnail, which would be captured from the level editor's current viewport.   (Edit > Project Settings > Prefabricator > Show Asset Thumbnails)

Version 1.2.3
-------------
* New: Added UE 4.24 support

Version 1.2.2
-------------
* Fix: Undo no longer crashes the editor [@grujicbr]

Version 1.2.1
-------------
* New: Added 4.23 engine support
* New: While saving a prefab, the names of the individual actors are also saved so they are restored correctly when the prefab is spawned
* Fix: Added null pointer checks while serializing / deserializing prefabs to avoid crash issues with stale uobjects

Version 1.2.0
-------------
* New: Prefabs now store soft refernces to assets and the references can be viewed from the reference viewer. This keeps the prefab from breaking when the assets are moved around
* New: Added Upgrade option in the Prefabricator Tool button's Advanced submenu. This upgrades all the existing prefabs in the project to the latest version, adding soft references
* New: Project settings to control automatic updates of prefabs in the level [Contribution from @iniside] 
* New: Project settings to control default pivot location when new prefabs are created [Contribution from @iniside] 
* New: Save an existing prefab in the level as a new asset [Contribution from @iniside] 
* New: Unlink prefabs in the scene to turn them into normal actors
* Fix: Fixed prefab collection loading issues in non-editor builds which was causing randomization to not work


Version 1.1.0
-------------
* New: Added 4.22 engine support
* New: Added class exports in C++ to make them visible outside of the module
* New: Mobility of the prefab actor is now editable from the Details window


Version 1.0.5
-------------
* Prefab Collection asset can be created by right clicking on multiple selected prefab assets
* Fixed an issue where detail panel buttons on the multiple selected prefab actors were working only on the first selected actor
* Added version number to the serialized prefab asset and asset collection
* Updated prefab collection asset data structure to use soft object pointer instead of direct uobject reference of prefab assets

Version 1.0.4
-------------
* Fixed a mobility bug with prefabs
* Added whitelist platforms to the plugin descriptor as per the marketplace requirements
* Added categories to all blueprint UFUNCTIONs so it doesn't cause compile errors on engine source builds
* Added authors page
* Added undo transaction on prefab creation

Version 1.0.3
-------------
* Added whitelist platforms to the plugin descriptor as per the marketplace requirements

Version 1.0.2
-------------
* Added Marketplace url in the plugin descriptor file
* Updated the year to 2019 in copyright header of the C++ files as per the marketplace guidelines

Version 1.0.1
-------------
* Added user guide and other support links in the Prefab menu
* Added documentation url in the plugin descriptor

Version 1.0.0
-------------
* Initial Version. Visit http://prefabricator.io for feature list
