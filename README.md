#aug                                                                                                                               
__aug__ is an extra terminal emulation layer to __aug__ment 
terminal functionality.

##project status
April 2013: 
	__aug__ is currently beta software and is under active development.
	I'm currently focusing on improving documentation for the project.

##overview
Similar to screen or tmux, __aug__ facilitates transparent control of a terminal
screen between terminal applications (such as bash, vim or emacs) and the
terminal emulator (such as xterm). The difference is that while tmux and screen
primarily provide terminal multiplexing funtionality, __aug__ attempts only to
provide hooks to a loaded __aug__ plugin so that it may access the terminal
screen and keyboard input.

For example, one could implement an __aug__ plugin which provides terminal
multiplexing functionality (though this would be somewhat pointless, as tmux and
screen already do a great job at this), but __aug__ does not provide this
type of functionality by default.

The following example plugins can be found in the `./plugins` directory
of the source tree:

 * __hello__: 	This is a minimal 'hello world' plugin.
 * __bold__: 	This example demonstrates how a plugin can hook the process by
				which __aug__ writes terminal characters to the terminal screen.
				It modifies the ncurses attributes of each character written to
				the screen to have the 'bold' attribute.
 * __rainbow__: Similar to the bold example, this plugin changes the foreground
				color of each character written to the screen to a random color.
 * __reverse__: Unlike 'bold' and 'rainbow' this plugin modifies more than just
				character attributes. It demonstrates that the entire screen 
				can be controlled using the __aug__ API by reversing the 
				orientation of the terminal screen. This plugin is quite useful
				for April 1st shenanigans :D

These plugins are of course pointless, they are only meant as small examples. For
a more complex example, please see the code at [aug-db](https://github.com/cantora/aug-db).

##installation

###requirements 
 * [__libvterm__](https://code.launchpad.net/~leonerd/libvterm)  
		__aug__ uses libvterm for all its terminal emulation needs. The make file
		will use the `bzr` (bazaar) utility to checkout a copy of libvterm from
		launchpad, so if you don't have `bzr` installed you either have to install
		it or download libvterm into a `libvterm` directory in the root directory
		of the __aug__ source tree.
 * __ncursesw__ (with developer headers)  
		__aug__ uses ncurses for screen manipulation and requires the 'wide
		character' version in order to support UTF character sets. In a Debian 
		based Linux distribution, you can install ncursesw with development headers
		using the following command `sudo apt-get install libncursesw5-dev`.

###compilation

First checkout the source tree using `git` (or alternatively, download the zip archive)
and `cd` to the root directory of the source tree. Now run the `make` command to
compile the code. If the compilation is successful, you should find the compiled binary
`aug` in the current directory. If you want to install it into your system, you can
simply copy the binary to an appropriate system install path or to your ~/bin
directory ( __aug__ doesnt require any other resource files to be installed into the
system in order to run).

If compilation fails, please create an 'issue' at the github project page for __aug__
and I'll do my best to respond quickly and help you resolve it.

##contribution
Contributions are of course welcome and greatly appreciated. If you have trouble
building the software and find that you have to do something special in order to compile
correctly, please submit information about what you did so I can incorporate it into
the makefile.

##license

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.html). See LICENSE or the given url for
details.
