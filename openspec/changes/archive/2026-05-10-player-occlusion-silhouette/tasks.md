## 1. Occluder identification

- [x] 1.1 Keep the existing y-sorted overworld render queue and add helper logic that identifies which generated-content entries render in front of the player for the active frame.
- [x] 1.2 Add focused tests for the queue-derived front-occluder selection so the silhouette source stays aligned with normal world draw order.

## 2. Silhouette compositing

- [x] 2.1 Add an offscreen occluder mask and player silhouette composite pass in the SFML shell using the existing player sprite transforms and front-object sprite transforms.
- [x] 2.2 Tune the silhouette presentation so only the hidden portion of the player is tinted/faded while the visible portion of the normal sprite remains unchanged.

## 3. Integration and validation

- [x] 3.1 Integrate the silhouette pass into the overworld render flow without breaking terrain rendering or the baseline global y-sort behavior.
- [x] 3.2 Add or update tests covering unchanged unoccluded player rendering and run the existing build/test suite.
