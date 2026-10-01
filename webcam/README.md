# Quick experiments in grabbing data from the webcam

Generated (almost) completely by Gemini search.

## [capture.cpp](capture.cpp) 

> Can you give me a c/c++  program for linux to grab one frame from my webcam
> and dump it to jpeg?

It made a minor typo

> I get:  error: no member named 'mgoffset' in 'v4l2_buffer'

Told me the one character typo fix (`mgoffset` -> `m.offset`) and gave a
detailed explanation what the mistake was. Program compiled and ran perfectly.

(14 kB binary)


## [capture_and_show.cpp](capture_and_show.cpp) 

> Awesome. Could you add code to show this jpg in a window as simply as
> possible?

It gave me the correct link for `stb_image.h` download in the description, but
then handed me a completely botched `wget` command. Eh. Code was perfect and
worked in one shot. 

(150 kB binary)

## [stream_preview.cpp)(stream_preview.cpp)

> Superb! Could you modify this program so that it loops, taking snapshots at
> regular intervals until the window is closed?

Code was perfect and worked in one shot. Gave brief and precise notes on what
the changes were and why we need them.

(151 kB binary)
