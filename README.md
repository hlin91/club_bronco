<h1>club_bronco</h1>
<img src=https://cdn.discordapp.com/attachments/750527778062991404/779156385283702804/unknown.png height=50% width=50%>
<h2>Build Instructions</h2>
<p>
  <strong>IMPORTANT: </strong>The game is configured to be purely locally hosted by default. To change this, edit config.h first and set <code>NO_FRIENDS</code> to false. Then, change <code>IP_ADDRESS</code> to the public IP of the computer hosting the server, <code>IP_ADDRESS_LOCAL</code> to the local IP of the hosting computer and, <code>PORT</code> to the port number to be used for client-server communications.<br><br>
  Navigate to the root directory and run the Makefile with <code>make</code>. Start the server with <code>./server</code> and the client with <code>./client</code>.
</p>
<h2>Configuration</h2>
<p>
  To change the public IP address the server is hosted on, edit <code>IP_ADDRESS</code> in config.h.<br>
  To change the local IP address the server listens on, edit <code>IP_ADDRESS_LOCAL</code> in config.h.
</p>
<h2>Supported Platforms</h2>
<p>
  Linux and OSX 10.15 or above<br>
</p>
<h2>OLC Game Engine</h2>
If <code>make</code> fails to build olcPixelGameEngine for some reason, reference the olcPixelGameEngine documentation below.<br>
<a href="https://github.com/OneLoneCoder/olcPixelGameEngine/wiki">Information</a>
<br>
<a href="https://github.com/OneLoneCoder/olcPixelGameEngine/wiki/Compiling-on-Linux">Compilation on Linux</a>
<br>
<a href="https://github.com/OneLoneCoder/olcPixelGameEngine/wiki/Compiling-with-Visual-Studio">Compilation on Windows</a>
<h2>Controls</h2>
<ul>
  <li>Click to move</li>
  <li>Enter to type and send messages</li>
  <li>Quit with esc</li>
</ul>
