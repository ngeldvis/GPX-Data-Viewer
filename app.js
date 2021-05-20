'use strict'

var DBConfig

// C library API
const ffi = require('ffi-napi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');
const mysql = require('mysql2/promise');

app.use(fileUpload());
app.use(express.static(path.join(__dirname+'/uploads')));
// app.use(express.json());

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {

    if(!req.files) {
        console.log('test1');
        return res.status(500).send('No file uploaded');
    }
    
    let uploadFile = req.files.uploadFile;
    let filename = uploadFile.name;
    
    let ext = '.gpx';
    fs.readdir('./uploads', (err, files) => {
        if(err) {
            return res.status(500).send('No file uploaded');
        } else {
            let gpxfiles = [];
            files.forEach(file => {
                if(path.extname(file) == ext) {
                    gpxfiles.push(file);
                }
            })
            for(var i = 0; i < gpxfiles.length; i++) {
                if(gpxfiles[i] === filename) {
                    return res.status(500).send('Attempt to upload duplicate file');
                }
            }
            // Use the mv() method to place the file somewhere on your server
            uploadFile.mv('uploads/' + uploadFile.name, function(err) {
                if(err) {
                    return res.status(500).send(err);
                }
                if(sharedLib.validateFilename('./uploads/'+filename, 'gpx.xsd') == true) {
                    return res.status(200).send(filename);
                } else {
                    return res.status(500).send('Attempt to upload invalid file');
                }
            });
        }
    });

});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res) {
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      console.log('Error in file downloading route: '+err);
      res.send('');
    }
  });
});

// ******************** Your code goes here ******************** //

var sharedLib = ffi.Library('./libgpxparser', {
    'createGPXJSON': ['string', ['string', 'string']],
    'createTracklistJSON': ['string', ['string', 'string']],
    'createRoutelistJSON': ['string', ['string', 'string']],
    'getDetailedTrackInfo': ['string', ['string', 'string', 'string']],
    'getDetailedRouteInfo': ['string', ['string', 'string', 'string']],
    'validateFilename': ['bool', ['string', 'string']],
    'jsonToWriteGPXdoc': ['bool', ['string', 'string', 'string']],
    'changeTrackName': ['bool', ['string', 'string', 'int']],
    'changeRouteName': ['bool', ['string', 'string', 'int']],
    'addRouteToFile': ['bool', ['string', 'string']],
    'filenameToNumRoutes': ['int', ['string']],
    'addWaypointToRoute': ['bool', ['string', 'int', 'string']],
    'fileToJSONTracksBetween': ['string', ['string', 'float', 'float', 'float', 'float', 'float']],
    'fileToJSONRoutesBetween': ['string', ['string', 'float', 'float', 'float', 'float', 'float']],
    'fileToGetRoutesOfLength': ['int', ['string', 'float']],
    'fileToGetTracksOfLength': ['int', ['string', 'float']],
    'RouteToWptList': ['string', ['string', 'int']]
});

app.get('/getGPXFiles', (req, res) => {
    let ext = '.gpx';
    fs.readdir(req.query.dir, (err, files) => {
        if(err) {
            console.log(err);
        } else {
            let gpxfiles = [];
            files.forEach(file => {
                if(path.extname(file) == ext) {
                    gpxfiles.push(file);
                }
            })
            res.send({
                files: gpxfiles
            });
        }
    });
});

app.get('/createGPXJSON', (req, res) => {
    let rv = sharedLib.createGPXJSON(req.query.file, 'gpx.xsd');
    res.send(rv);
});

app.get('/createTracklistJSON', (req, res) => {
    let rv = sharedLib.createTracklistJSON('./uploads/'+req.query.file, 'gpx.xsd');
    res.send(rv);
});

app.get('/createRoutelistJSON', (req, res) => {
    let rv = sharedLib.createRoutelistJSON('./uploads/'+req.query.file, 'gpx.xsd');
    res.send(rv);
});

app.get('/getDetailedTrackInfo', (req, res) => {
    let rv = sharedLib.getDetailedTrackInfo('./uploads/'+req.query.file, 'gpx.xsd', req.query.trackname);
    res.send(rv);
});

app.get('/getDetailedRouteInfo', (req, res) => {
    let rv = sharedLib.getDetailedRouteInfo('./uploads/'+req.query.file, 'gpx.xsd', req.query.routename);
    res.send(rv);
});

app.get('/changeTrackName', (req, res) => {
    let rv = sharedLib.changeTrackName('./uploads/'+req.query.file, req.query.name, req.query.id);
    res.send({
        value: rv
    });
});

app.get('/changeRouteName', (req, res) => {
    let rv = sharedLib.changeRouteName('./uploads/'+req.query.file, req.query.name, req.query.id);
    res.send({
        value: rv
    });
});

app.post('/createGPX', (req, res) => {
    // format: {"version":ver,"creator":"creatorValue"}
    let creator = req.body.creator;
    let version = req.body.version;
    let filename = req.body.filename;
    let jsonText = '{"version":'+version+',"creator":"'+creator+'"}';
    let rv = sharedLib.jsonToWriteGPXdoc(jsonText, './uploads/'+filename, 'gpx.xsd');
    if(rv == false) {
        return res.status(500).send('File must end with .gpx extension');
    }
    return res.status(200).send(filename);
});

app.post('/addRoute', (req, res) => {
    console.log(req.body);
    let name = req.body.name;
    if(name == '') {
        name = ' ';
    }
    let filename = req.body.filename;
    let routeJSON = '{"name":"'+name+'"}'
    let rv = sharedLib.addRouteToFile('./uploads/'+filename, routeJSON);
    if(rv == false) {
        console.log('error');
    }
    res.redirect('/');
});

app.get('/getNumRoutes', (req, res) => {
    let rv = sharedLib.filenameToNumRoutes('./uploads/'+req.query.file);
    res.send({
        value: rv
    });
});

app.post('/addWaypoint', (req, res) => {
    // format: {"lat":latVal,"lon":lonVal}
    console.log(req.body);
    let wptJSON = '{"lat":'+req.body.lat+',"lon":'+req.body.lon+'}'
    let rv = sharedLib.addWaypointToRoute('./uploads/'+req.body.filename, req.body.route, wptJSON);
    if(rv == false) {
        console.log('error');
    }
    res.redirect('/');
});

app.get('/getPathsBetween', (req, res) => {
    let filename = req.query.file;

    let lat1 = req.query.la1;
    let lat2 = req.query.la2;
    let lon1 = req.query.lo1;
    let lon2 = req.query.lo2;
    let d = req.query.delta;
    
    let trks = sharedLib.fileToJSONTracksBetween('./uploads/'+filename, lat1, lon1, lat2, lon2, d);
    let rtes = sharedLib.fileToJSONRoutesBetween('./uploads/'+filename, lat1, lon1, lat2, lon2, d);

    res.send({
        tracks: trks,
        routes: rtes
    });
});

app.get('/validateFile', (req, res) => {
    let rv = sharedLib.validateFilename('./uploads/'+req.query.file, 'gpx.xsd');
    res.send({
        value: rv
    });
});

app.get('/getNumRoutesOfLength', (req, res) => {
    console.log('test2');
    let rv = sharedLib.fileToGetRoutesOfLength('./uploads/'+req.query.file, req.query.len);
    console.log('test3');
    res.send({
        val: rv
    });
})

app.get('/getNumTracksOfLength', (req, res) => {
    let rv = sharedLib.fileToGetTracksOfLength('./uploads/'+req.query.file, req.query.len);
    res.send({
        val: rv
    });
})

app.post('/login', async (req, res) => {

    let config = {
        host     : req.body.host,
        user     : req.body.user,
        password : req.body.password,
        database : req.body.database
    };

    DBConfig = config
    
    let connection;
    try {

        connection = await mysql.createConnection(DBConfig);
        
        // create tables

        await connection.execute('CREATE TABLE IF NOT EXISTS FILE(  \
          gpx_id INT AUTO_INCREMENT PRIMARY KEY,  \
          file_name VARCHAR(60) NOT NULL,     \
          ver DECIMAL(2,1) NOT NULL,  \
          creator VARCHAR(256) NOT NULL   \
        )');

        await connection.execute('CREATE TABLE IF NOT EXISTS ROUTE( \
          route_id INT AUTO_INCREMENT PRIMARY KEY,    \
          route_name VARCHAR(256),    \
          route_len FLOAT(15,7) NOT NULL, \
          gpx_id INT NOT NULL,    \
          FOREIGN KEY(gpx_id) REFERENCES FILE(gpx_id) ON DELETE CASCADE   \
        )');

        await connection.execute('CREATE TABLE IF NOT EXISTS POINT( \
          point_id INT AUTO_INCREMENT PRIMARY KEY,    \
          point_index INT NOT NULL,   \
          latitude DECIMAL(11,7) NOT NULL,    \
          longitude DECIMAL(11,7) NOT NULL,   \
          point_name VARCHAR(256),    \
          route_id INT NOT NULL,  \
          FOREIGN KEY(route_id) REFERENCES ROUTE(route_id) ON DELETE CASCADE  \
        )');

        let files = (await connection.execute('SELECT file_name FROM FILE'))[0];

        res.send({
            files: files,
            status: true
        });
    } catch(e) {
        console.log("Query error: "+e);
        res.send({
            status: false
        });
    } finally {
        if (connection && connection.end) connection.end();
    }
});

app.get('/storeAllFiles', async (req, res) => {

    // FILE format: gpx_id, filename, version, creator 
    // ROUTE format: route_id, route_name, route_len, gpx_id 
    // POINT format: point_id, point_index, latitude, longitude, point_name, route_id

    let files = req.query.files;

    let connection;
    try {

        connection = await mysql.createConnection(DBConfig);

        /// Insert Files
        for(var i = 0; i < files.length; i++) {
            let file = files[i];
            let fileJSONstr = sharedLib.createGPXJSON(file, 'gpx.xsd');
            let fileJSON = JSON.parse(fileJSONstr);

            let query = 'INSERT INTO FILE (file_name, ver, creator) VALUES ("'+file+'", "'+fileJSON.version+'", "'+fileJSON.creator+'")';
            await connection.execute(query);
            let gpx_id = (await connection.execute('SELECT LAST_INSERT_ID()'))[0][0]['LAST_INSERT_ID()'];

            
            let rtes = sharedLib.createRoutelistJSON('./uploads/'+file, 'gpx.xsd');
            let rteList = JSON.parse(rtes);
            
            /// Insert Routes
            for(var j = 0; j < rteList.length; j++) {
                let rte = rteList[j];


                let query2 = 'INSERT INTO ROUTE (route_name, route_len, gpx_id) VALUES ('+(rte.name == 'None' || rte.name == ' ' ? 'null' : '"'+rte.name+'"')+', "'+rte.len+'", "'+gpx_id+'")';
                await connection.execute(query2);
                let route_id = (await connection.execute('SELECT LAST_INSERT_ID()'))[0][0]['LAST_INSERT_ID()'];
                
                let wpts = sharedLib.RouteToWptList('./uploads/'+file, j+1);
                let wptList = JSON.parse(wpts);
                
                /// Insert Route Waypoints
                for(var k = 0; k < wptList.length; k++) {
                    let wpt = wptList[k];

                    let query3 = 'INSERT INTO POINT (point_index, latitude, longitude, point_name, route_id) VALUES ("'+k+'", "'+wpt.lat+'", "'+wpt.lon+'", '+(wpt.name == 'None' ? 'null': '"'+wpt.name+'"')+', "'+route_id+'")';
                    await connection.execute(query3);
                }
            }
        }

        res.send({
            status: true
        });
    } catch(e) {
        console.log("Query error: "+e);
        res.send({
            status: false
        });
    } finally {
        if (connection && connection.end) connection.end();
    }
});

app.get('/query1', async (req, res) => {
    let namesort = false;
    if(req.query.sortvalue == 'name') {
        namesort = true;
    }
    let connection;
    try {
        connection = await mysql.createConnection(DBConfig);
        var data = (await connection.execute('SELECT * FROM ROUTE ORDER BY ' + (namesort == true ? 'route_name' : 'route_len')))[0];
        for(var i = 0; i < data.length; i++) {
            let filename = (await connection.execute('SELECT file_name FROM FILE WHERE gpx_id = ' + data[i].gpx_id))[0][0].file_name;
            data[i].file = filename;
        }
        res.send(data);
    } catch(e) {
        console.log('Query error: ' + e);
        res.send({status: false});
    } finally {
        if (connection && connection.end) connection.end();
    }
});

app.get('/query2', async (req, res) => {
    let sortbyname = false;
    if(req.query.sortvalue == 'name') {
        sortbyname = true;
    }
    let file = req.query.file;
    let connection;
    try {

        connection = await mysql.createConnection(DBConfig);

        let gpx_id = (await connection.execute('SELECT gpx_id FROM FILE WHERE file_name = "'+file+'"'))[0][0].gpx_id;

        if(sortbyname == true) {
            var data = (await connection.execute('SELECT * FROM ROUTE WHERE gpx_id = '+gpx_id+' ORDER BY route_name'))[0];
        } else { // sort by length
            var data = (await connection.execute('SELECT * FROM ROUTE WHERE gpx_id = '+gpx_id+' ORDER BY route_len'))[0];
        }

        res.send(data);
    } catch(e) {
        console.log("Query error: "+e);
        res.send({
            status: false
        });
    } finally {
        if (connection && connection.end) connection.end();
    }
});

app.get('/query3', async (req, res) => {
    let rte = req.query.route;
    let file = req.query.file;

    let connection;
    try {

        connection = await mysql.createConnection(DBConfig);

        let gpx_id = (await connection.execute('SELECT gpx_id FROM FILE WHERE file_name = "'+file+'"'))[0][0].gpx_id;
        let route_id = (await connection.execute('SELECT route_id FROM ROUTE WHERE gpx_id = '+gpx_id))[0][rte-1].route_id;

        var data = (await connection.execute('SELECT * FROM POINT WHERE route_id = '+route_id+' ORDER BY point_index'))[0];

        res.send(data);
    } catch(e) {
        console.log('Query error: '+e);
        res.send({
            status: false
        });
    } finally {
        if (connection && connection.end) connection.end();
    }
});

app.get('/query4', async (req, res) => {
    let sortbyname = false;
    if(req.query.sortvalue == 'name') {
        sortbyname = true;
    }
    let file = req.query.file;
    let connection;
    try {

        connection = await mysql.createConnection(DBConfig);

        let gpx_id = (await connection.execute('SELECT gpx_id FROM FILE WHERE file_name = "'+file+'"'))[0][0].gpx_id;      
        let routes;
        if(sortbyname == true) {
            routes = (await connection.execute('SELECT route_id FROM ROUTE WHERE gpx_id = '+gpx_id+' ORDER BY route_name'))[0];
        } else { // sort by route length
            routes = (await connection.execute('SELECT route_id FROM ROUTE WHERE gpx_id = '+gpx_id+' ORDER BY route_len'))[0];
        }
        console.log(routes);

        let rtes = [];
        for(var i = 0; i < routes.length; i++) {
            let route_id = routes[i].route_id;
            let route_name = (await connection.execute('SELECT route_name FROM ROUTE WHERE route_id = '+route_id))[0][0].route_name;
            let rte = {id: route_id, name: route_name};
            rtes.push(rte);
        }
        
        let data = [];
        for(i = 0; i < rtes.length; i++) {
            let point_data = (await connection.execute('SELECT * FROM POINT WHERE route_id = '+rtes[i].id+' ORDER BY point_index'))[0];
            let dataset = {data: point_data, route: rtes[i].name}
            data.push(dataset);
        }

        res.send(data);
    } catch(e) {
        console.log("Query error: "+e);
        res.send({
            status: false
        });
    } finally {
        if (connection && connection.end) connection.end();
    }
});

app.get('/query5', async (req, res) => {
    
    let shortest = false;
    if(req.query.type == 'short') {
        shortest = true;
    }
    let file = req.query.file;
    let n = req.query.n_value;

    let connection;
    try {

        connection = await mysql.createConnection(DBConfig);

        let gpx_id = (await connection.execute('SELECT gpx_id FROM FILE WHERE file_name = "'+file+'"'))[0][0].gpx_id;

        if(shortest == true) {
            var result = (await connection.execute('SELECT * FROM ROUTE WHERE gpx_id = '+gpx_id+' ORDER BY route_len'))[0];
        } else { // longest
            var result = (await connection.execute('SELECT * FROM ROUTE WHERE gpx_id = '+gpx_id+' ORDER BY route_len DESC'))[0];
        }

        let data = [];
        for(var i = 0; i < Math.min(n, result.length); i++) {
            data.push(result[i]);
        }

        res.send(data);
    } catch(e) {
        console.log("Query error: "+e);
        res.send({
            status: false
        });
    } finally {
        if (connection && connection.end) connection.end();
    }
});

app.post('/clearData', async (req, res) => {
    let connection;
    try {
        connection = await mysql.createConnection(DBConfig);
        await connection.execute('DELETE FROM FILE');
        await connection.execute('ALTER TABLE FILE AUTO_INCREMENT=1');
        await connection.execute('ALTER TABLE ROUTE AUTO_INCREMENT=1');
        await connection.execute('ALTER TABLE POINT AUTO_INCREMENT=1');
        res.send(true);
    } catch(e) {
        console.log('Query error: '+e);
        res.send(false);
    } finally {
        if (connection && connection.end) connection.end();
    }
});

app.get('/DBstatus', async (req, res) => {
    let shortest = false;
    if(req.query.type == 'short') {
        shortest = true;
    }
    let file = req.query.file;
    let n = req.query.n_value;

    let connection;
    try {

        connection = await mysql.createConnection(DBConfig);

        let num_files = (await connection.execute('SELECT COUNT(*) FROM FILE'))[0][0]['COUNT(*)'];
        let num_routes = (await connection.execute('SELECT COUNT(*) FROM ROUTE'))[0][0]['COUNT(*)'];
        let num_points = (await connection.execute('SELECT COUNT(*) FROM POINT'))[0][0]['COUNT(*)'];

        let data = {files: num_files, routes: num_routes, points: num_points};

        res.send(data);
    } catch(e) {
        console.log("Query error: "+e);
        res.send({
            status: false
        });
    } finally {
        if (connection && connection.end) connection.end();
    }
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);
