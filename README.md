#aug
aug is an extra terminal emulation layer to augment 
terminal functionality.

##project status
April 2013: 
	aug is currently beta software and is under active development.
	I'm currently focusing on improving documentation for the project.

##overview
Similar to screen or tmux, aug facilitates transparent control of a terminal
screen between terminal applications (such as bash, vim or emacs) and the
terminal emulator (such as xterm). The difference is that while tmux and screen
primarily provide terminal multiplexing funtionality, aug attempts only to
provide hooks to a loaded aug plugin so that it may access the terminal
screen and keyboard input.

For example, one could implement an aug plugin which provides terminal
multiplexing functionality (though this would be somewhat pointless, as tmux and
screen already do a great job at this), but aug does not provide this
type of functionality by default.

The following example plugins can be found in the `./plugins` directory
of the source tree:

 * __hello__: 	This is a minimal 'hello world' plugin.
 * __bold__: 	This example demonstrates how a plugin can hook the process by
				which aug writes terminal characters to the terminal screen.
				It modifies the ncurses attributes of each character written to
				the screen to have the 'bold' attribute.
 * __rainbow__: Similar to the bold example, this plugin changes the foreground
				color of each character written to the screen to a random color.
 * __reverse__: Unlike 'bold' and 'rainbow' this plugin modifies more than just
				character attributes. It demonstrates that the entire screen 
				can be controlled using the aug API by reversing the 
				orientation of the terminal screen. This plugin is quite useful
				for April 1st shenanigans :D

These plugins are of course pointless, they are only meant as small examples. For
a more complex example, please see the code at [aug-db](https://github.com/cantora/aug-db).

##how does it work?
When `aug` is invoked from the terminal, it opens a new pseudo-terminal (pty) 
connected to a child process (usually a shell such as `bash`). Any keyboard
input from the original terminal is passed along to the child's pty and any
terminal output from the child is interpreted by libvterm and used to control
the original terminal (using the ncurses library). Since aug acts as an 
intermediary for these I/O processes, it is able to provide hooks for plugins
to modify/enhance/automate the keyboard input and/or the terminal output.

##documentation
See the [wiki](https://github.com/cantora/aug/wiki/_pages) for documentation on aug.
If you don't want to view the wiki documentation in your browswer, you can checkout
a copy of the pages by running `git clone git://github.com/cantora/aug.wiki.git`.  

In addition to the wiki, you can find detailed instructions on using the plugin API
in the comments of `./include/aug.h`.

##installation
Please see the [installation page](https://github.com/cantora/aug/wiki/Installation)
in the wiki for detailed instructions. If you have all the required dependencies 
installed, simply running the `make` command in the root of the source tree should
do the trick. The compilation process generates a single binary called `aug`, so 
to install you can simply copy the binary to some location that is in your PATH.

##contribution
Contributions are of course welcome and greatly appreciated. If you have trouble
building the software and find that you have to do something special in order to compile
correctly, please submit information about what you did so I can incorporate it into
the makefile.

##license
[GPLv3](http://www.gnu.org/licenses/gpl-3.0.html). See LICENSE or the given url for
details.
