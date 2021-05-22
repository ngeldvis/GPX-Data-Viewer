# GPX Data Viewer

A web app to display GPX data from GPX files uploaded to a server. This was a school project I completed in second year in my Software Systems Development and Integration ([CIS 2750](https://www.uoguelph.ca/registrar/calendars/undergraduate/2018-2019/courses/cis2750.shtml)) course

### Main Acomplishments

* Built a web user interface using HTML, CSS, and Javascript to display GPX data to the user
* Implemented a back-end server using C, Javascript, and Node.js used to parse GPX files allowing the user to make requests for information stored in files uploaded to the server
* Connected the server to a MySQL relational database used to store and share GPX file information

### What is GPX?

GPX is an Extensible Markup Language (XML) GPS data format used in software applications to describe waypoints, tracks, and routes. You can learn more [here](https://www.topografix.com/gpx.asp)

### Running the App

First you need to make sure that you have node.js and npm installed on your system. Then enter `npm install` to install all the necessary dependencies. Finally to run the server enter `npm run dev PORTNUM` so that you can access the application at `localhost:PORTNUM`
