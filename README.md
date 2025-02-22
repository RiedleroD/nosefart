Nosefart plays [NSF](http://vgmpf.com/Wiki/index.php?title=NSF) files.

This program has changed hands many many times over the years and each person mostly just changed what they needed atm. This means it's generally buggy and unreliable, but I'm trying my best to change that.

Because it's had such a long heritage, many many variants of nosefart exist. From a gtk standalone player, to a winamp plugin, and many more. The code has been borrowed by other projects, and it borrows code from yet other projects.
In any case, this repository contains only what's necessary for the linux standalone command-line tool, which was always the primary target. Other continuations of nosefart might still include more things, but are usually much more outdated.

## Usage

```
Usage: nosefart [OPTIONS] filename
Play an NSF (NES Sound Format) file.

Options:
	-h  	Help
	-v  	Version information

	-t x	Play track x (default: 1)
	-s x	Play at x times the normal speed
	-f x	Use x sampling rate (default: 44100)
	-B x	Use sample size of x bits (default: 8)
	-l x	Limit total playing time to x seconds (0 = unlimited)
	-r x	Limit total playing time to x frames (0 = unlimited)
	-b x	Skip the first x frames
	-a x	Calculate song length and play x repetitions (0 = intro only)
	-i	Just print file information and exit (specify twice for subsong lengths)
	-x	Start with channel x disabled (-123456)
	-o x	Output WAV file(s) to directory x
```

Nosefart uses SDL2, so make sure you have that.

## Credits

- Matthew Conte wrote the sound core and the original Linux and DOS programs.
  He says that nosefart was "inspired by the MSP 0.50 source release by Sevy and Marp \[and\] ported by Neil Stevens."
- Matthew Strait updated the Linux command line program and added gnosefart.
- Benjamin Gerard contributed the code that calculates song length.
- K`rai (a.k.a HJ) has fixed some bugs and worked on the Winamp plugin.
- Raphael Assenat made Linux nosefart work on big endian machines.
- Misterhat extracted core nosefart, ported it to SDL2 & added wav dumping capability.
- Riedler did lots of things, mostly fixing bugs and cleanup (see git log)
- Some other people are probably forgotten. :-(