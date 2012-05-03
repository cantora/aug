aug
===
a screen layer on top of the shell to augment terminal functionality.
this just an invisible (at best) layer that does nothing right now.
   
compilation:

 - uses libvterm (https://code.launchpad.net/~leonerd/libvterm) for all the 
   terminal escape parsing stuff. the make file will use the bzr utility
   to checkout a copy of libvterm from launchpad, so if you dont have 
   bzr installed you have to create the ./libvterm directory with all the 
   required contents in some other way.
 - ncursesw required for all the screen manipulation

 