# Custom Captions

A tool to pair specific times in an audio file with custom text.

## Instructions

Load an image.

Mark the image.

Enter text.

Move a marker.

Delete a marker.

Save/export data.

## Contribution

This is for any devs who wish to contribute. Currently, I'm storing any used libraies in the repo (it's not the best solution, but I won't be changing it till I have a build pipeline for those).

### Current Goals

- [x] can dock windows within application
- [x] create new project with audio file
- [x] use ffmpeg to turn audio file into waveform image
- [x] display audio waveform image in window
- [x] fit waveform image to window
- [x] add visual markers over waveform image
- [x] edit visual markers via click & drag
- [x] ensure markers stay in order of their time value
- [x] ensure new markers cannot be inserted tightly between others
- [x] ensure markers do not exceed their neighbors bounds
  - issue: lots of jitter; can currently override
- [x] remove markers with left click
- [X] create window for text inputs
- [X] add input box for each marker
- [x] insert input box between markers
- [x] delete input box when removing marker
- [x] play/pause button
- [x] zoom in an out of image
- [ ] serialize data in time/text pairs (json is fine)
- [ ] load project with project data
- [ ] save project data
- [ ] open project data
- [ ] hovering/scrolling in one window will cause the other to match
