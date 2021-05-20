
var isTrack = false;
var trackVal = -1;
var routeVal = -1;

// Put all onload AJAX calls here, and event listeners
jQuery(document).ready(function() {

    // create database tables

    $('#loginForm').submit((event) => {
        event.preventDefault();

        let form = new FormData($('#loginForm')[0]);
        jQuery.ajax({
            type: 'post',
            async: false,
            encType: 'multipart/form-data',
            url: '/login',
            data: form,
            processData: false, 
            contentType: false,
            success: (data) => {
                if(data.status == false) {
                    alert('Credentials Failed');
                } else {
                    console.log('Logged in Successfully')
                    $('#login-background, #loginForm').hide();
                    $('#database-stuff').show();
                    for(var i = 0; i < data.files.length; i++) {
                        addToDBFileList(data.files[i].file_name);
                    }
                }
            },
            fail: (err) => {
                alert(err);
                console.log(err);
            },
            error: (err) => {
                alert(err.responseText);
                console.log(err.responseText);
            }
        });
    });

    // startup UI stuff

    $('.q-table').hide();
    $('.queryForm').hide();
    $('#renameForm').hide();
    $('#find-paths-table-container').hide();
    $('#tracks-found-title').hide();
    
    // Populate the Table of GPX files

    let files = getValidFiles();
    if(files.length > 0) {
        for(var i = 0; i < files.length; i++) {
            addToFilelogTable(files[i], i);
        }
    } else {
        $('#file-log-table').append('<tr class="none"><td colspan="6" class="center-text">No files</td></tr>');
        $('#store-all').hide();
    }

    $('#renameForm').submit((event) => {
        event.preventDefault();
        // need new name, track/route, track/route index, filename
        let newname = $('#rename-input').val();
        let filename = getGPXViewFile();
        if(isTrack == true) {
            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/changeTrackName',
                data: {
                    file: filename,
                    name: newname,
                    id: trackVal
                },
                success: function(data) {
                    if(data.value == false) {
                        alert('failed to change the name of the component');
                    } else {   
                        $('#track-'+trackVal+' td.name').text(newname);
                    }
                },
                fail: function(error) {
                    alert(error);
                    console.log(error);
                }
            });
        } else {
            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/changeRouteName',
                data: {
                    file: filename,
                    name: newname,
                    id: routeVal
                },
                success: function(data) {
                    if(data.value == false) {
                        alert('failed to change the name of the component');
                    } else { 
                        $('#route-'+routeVal+' td.name').text(newname);
                    }
                },
                fail: function(error) {
                    alert(error);
                    console.log(error);
                }
            });
        }
    });

    $('#findPathsForm').submit((event) => {

        event.preventDefault();

        let lat1 = $('#lat1-input').val();
        let lat2 = $('#lat2-input').val();
        let lon1 = $('#lon1-input').val();
        let lon2 = $('#lon2-input').val();
        let delta = $('#delta-input').val();

        $('#find-paths-table-container').show();
        $('#tracks-found-title').show();
        $('#find-paths-table tr').remove();
        
        let found = false;

        let files = getValidFiles();
        if(files.length > 0) {
            rteID = 1;
            trkID = 1;
            for(var i = 0; i < files.length; i++) {
                let filename = files[i];
                jQuery.ajax({
                    type: 'get',
                    dataType: 'json',
                    async: false,
                    url: '/getPathsBetween',
                    data: {
                        file: filename,
                        la1: lat1,
                        la2: lat2,
                        lo1: lon1,
                        lo2: lon2,
                        delta: delta
                    },
                    success: (data) => {

                        if(filename == 'minValid.gpx') {
                            console.log(data);
                        }

                        let routes = JSON.parse(data.routes);
                        let tracks = JSON.parse(data.tracks);
                        if(routes.length > 0 || tracks.length > 0) {
                            found = true;
                            // routes
                            for(var j = 0; j < routes.length; j++) {
                                let rte = routes[j];
                                let rowID = 'found-route-'+i+'-'+j;
                                
                                $('#find-paths-table').append('<tr id="'+rowID+'"></tr>');
                                $('#'+rowID).append('<td>'+filename+'</td>');
                                $('#'+rowID).append('<td>Route '+(rteID++)+'</td>');
                                if(rte.name === 'None') {
                                    $('#'+rowID).append('<td></td>');
                                } else {
                                    $('#'+rowID).append('<td>'+rte.name+'</td>');
                                }
                                $('#'+rowID).append('<td>'+rte.numPoints+'</td>');
                                $('#'+rowID).append('<td>'+rte.len+'</td>');
                                $('#'+rowID).append('<td>'+rte.loop+'</td>');
                            }
                            // tracks
                            for(var k = 0; k < tracks.length; k++) {
                                let trk = tracks[k];
                                let rowID = 'found-track-'+i+'-'+k;
    
                                $('#find-paths-table').append('<tr id="'+rowID+'"></tr>');
                                $('#'+rowID).append('<td>'+filename+'</td>');
                                $('#'+rowID).append('<td>Track '+(trkID++)+'</td>');
                                if(trk.name === 'None') {
                                    $('#'+rowID).append('<td></td>');
                                } else {
                                    $('#'+rowID).append('<td>'+trk.name+'</td>');
                                }
                                $('#'+rowID).append('<td>'+trk.numPoints+'</td>');
                                $('#'+rowID).append('<td>'+trk.len+'</td>');
                                $('#'+rowID).append('<td>'+trk.loop+'</td>');
                            }
                        }
                    },
                    fail: (error) => {
                        alert(error);
                        console.log(error);
                    }
                });
            }
            if(found === false) {
                $('#find-paths-table').append('<tr><td colspan="6" class="center-text">No Paths Found</td></tr>')
            }

        } else {
            alert('No file on server\nupload a file to use this feature')
        }

    });

    $('#uploadForm').submit((event) => {
        event.preventDefault();
        let form = new FormData($('#uploadForm')[0]);
        jQuery.ajax({
            type: 'post',
            async: false,
            encType: 'multipart/form-data',
            url: '/upload',
            data: form,
            processData: false, 
            contentType: false,
            success: (data) => {
                console.log('Uploaded '+data);
                location.reload();
            },
            fail: (err) => {
                console.log('test');
                alert(err);
                console.log(err);
            },
            error: (err) => {
                alert(err.responseText);
                console.log(err.responseText);
            }
        });
    });

    $('#createGPXForm').submit((event) => {
        event.preventDefault();
        let form = new FormData($('#createGPXForm')[0]);
        jQuery.ajax({
            type: 'post',
            encType: 'multipart/form-data',
            url: '/createGPX',
            data: form,
            processData: false, 
            contentType: false,
            success: (data) => {
                location.reload();
                console.log(data);
                console.log('created GPX file');
            },
            fail: (err) => {
                alert(err);
                console.log(err);
            },
            error: (err) => {
                alert(err.responseText);
                console.log(err.responseText);
            }
        });
    });

    $('#numRoutesForm').submit((event) => {
        console.log('test');
        event.preventDefault();
        let count = 0;
        let length = $('#getRoutesLength').val();
        let files = getValidFiles();
        console.log(files);
        for(var i = 0; i < files.length; i++) {
            let filename = files[i];
            jQuery.ajax({
                type: 'get',
                async: false,
                url: '/getNumRoutesOfLength',
                data: {
                    file: filename,
                    len: length
                },
                success: (data) => {
                    console.log(data.val);
                    count += data.val;
                },
                fail: (err) => {
                    alert(err);
                    console.log(err);
                },
                error: (err) => {
                    alert(err.responseText);
                    console.log(err.responseText);
                }
            });
        }
        alert('number of routes of length '+length+': '+count);
    });

    $('#numTracksForm').submit((event) => {
        console.log('test');
        event.preventDefault();
        let count = 0;
        let length = $('#getTracksLength').val();
        let files = getValidFiles();
        for(var i = 0; i < files.length; i++) {
            let filename = files[i];
            jQuery.ajax({
                type: 'get',
                async: false,
                url: '/getNumTracksOfLength',
                data: {
                    file: filename,
                    len: length
                },
                success: (data) => {
                    count += data.val;
                },
                fail: (err) => {
                    alert(err);
                    console.log(err);
                },
                error: (err) => {
                    alert(err.responseText);
                    console.log(err.responseText);
                }
            });
        }
        alert('number of tracks of length '+length+': '+count);
    });

    $('#query1Form').submit((event) => {
        event.preventDefault();
        let sortby = $('#query1Form input[type="radio"][name="sort"]:checked').val();
        jQuery.ajax({
            type: 'get',
            dataType: 'json',
            url: '/query1',
            data: {
                sortvalue: sortby
            },
            success: (data) => {
                $('#q1-table tr').remove();
                if(data.length == 0) {
                    $('#q1-table').append('<tr class="none"><td colspan="5" class="center-text">No routes</td></tr>');
                } 
                $('#q1-table-container').show();
                for(var i = 0; i < data.length; i++) {
                    let rte = data[i];

                    /*  <th>filename</th>
                        <th>route_name</th>
                        <th>route_len</th>  
                        <th>route_id</th>  
                        <th>gpx_id</th>  */

                    let rowID = 'q1-table-row'+i;
                    $('#q1-table').append('<tr id="'+rowID+'"></tr>');

                    $('#'+rowID).append('<td>'+rte.file+'</td>');
                    $('#'+rowID).append('<td>'+(rte.route_name == null ? '' : rte.route_name)+'</td>');
                    $('#'+rowID).append('<td>'+rte.route_len+'m</td>');
                    $('#'+rowID).append('<td>'+rte.route_id+'</td>');
                    $('#'+rowID).append('<td>'+rte.gpx_id+'</td>');
                }
            },
            fail: (err) => {
                alert(err);
                console.log(err);
            },
            error: (err) => {
                alert(err.responseText);
                console.log(err.responseText);
            }
        });
    });

    $('#query2Form').submit((event) => {
        event.preventDefault();
        let sortby = $('#query2Form input[type="radio"][name="sort"]:checked').val();
        let filename = getQuery2File();
        jQuery.ajax({
            type: 'get',
            dataType: 'json',
            url: '/query2',
            data: {
                sortvalue: sortby,
                file: filename
            },
            success: (data) => {
                $('#q2-table tr').remove();
                if(data.length == 0) {
                    $('#q2-table').append('<tr class="none"><td colspan="5" class="center-text">No routes</td></tr>');
                }
                $('#q2-table-container').show();
                for(var i = 0; i < data.length; i++) {
                    let rte = data[i];


                    /*  <th>filename</th>
                        <th>route_name</th>
                        <th>route_len</th>  
                        <th>route_id</th>  
                        <th>gpx_id</th>  */

                    let rowID = 'q2-table-row'+i;
                    $('#q2-table').append('<tr id="'+rowID+'"></tr>');

                    $('#'+rowID).append('<td>'+filename+'</td>');
                    $('#'+rowID).append('<td>'+(rte.route_name == null ? '' : rte.route_name)+'</td>');
                    $('#'+rowID).append('<td>'+rte.route_len+'m</td>');
                    $('#'+rowID).append('<td>'+rte.route_id+'</td>');
                    $('#'+rowID).append('<td>'+rte.gpx_id+'</td>');
                }
            },
            fail: (err) => {
                alert(err);
                console.log(err);
            },
            error: (err) => {
                alert(err.responseText);
                console.log(err.responseText);
            }
        });
    });

    $('#query3Form').submit((event) => {
        event.preventDefault();
        let filename = getQuery3File();
        let rte = $('#q3-route-picker').val();
        jQuery.ajax({
            type: 'get',
            dataType: 'json',
            url: '/query3',
            data: {
                file: filename,
                route: rte
            },
            success: (data) => {
                $('#q3-table tr').remove();
                if(data.length == 0) {
                    $('#q3-table').append('<tr class="none"><td colspan="6" class="center-text">No points</td></tr>');
                }
                $('#q3-table-container').show();
                for(var i = 0; i < data.length; i++) {
                    let wpt = data[i];

                    /*  <th>Point Index</th>
                        <th>Point Name</th>
                        <th>Latitude</th>
                        <th>Longitude</th>
                        <th>Point Id</th>
                        <th>Route Id</th> */

                    let rowID = 'q3-table-row'+i;
                    $('#q3-table').append('<tr id="'+rowID+'"></tr>');

                    $('#'+rowID).append('<td>'+wpt.point_index+'</td>');
                    $('#'+rowID).append('<td>'+(wpt.point_name == null ? '' : wpt.point_name)+'</td>');
                    $('#'+rowID).append('<td>'+wpt.latitude+'</td>');
                    $('#'+rowID).append('<td>'+wpt.longitude+'</td>');
                    $('#'+rowID).append('<td>'+wpt.point_id+'</td>');
                    $('#'+rowID).append('<td>'+wpt.route_id+'</td>');
                }
            },
            fail: (err) => {
                alert(err);
                console.log(err);
            },
            error: (err) => {
                alert(err.responseText);
                console.log(err.responseText);
            }
        });
    });

    $('#query4Form').submit((event) => {
        event.preventDefault();
        let sortby = $('#query4Form input[type="radio"][name="sort"]:checked').val();
        let filename = getQuery4File();
        jQuery.ajax({
            type: 'get',
            dataType: 'json',
            url: '/query4',
            data: {
                sortvalue: sortby,
                file: filename
            },
            success: (data) => {
                $('#q4-table tr').remove();
                if(data.length == 0) {
                    $('#q4-table').append('<tr class="none"><td colspan="6" class="center-text">No routes</td></tr>');
                }
                $('#q4-table-container').show();
                for(var i = 0; i < data.length; i++) {
                    let rte = data[i];
                    if(rte.route == null) {
                        $('#q4-table').append('<tr class="none"><td colspan="6" class="bold">Route '+(i+1)+': Unnamed</td></tr>');
                    } else {
                        $('#q4-table').append('<tr class="none"><td colspan="6" class="bold">Route '+(i+1)+': '+rte.route+'</td></tr>');
                    }

                    if(rte.data.length == 0) {
                        $('#q4-table').append('<tr class="none"><td colspan="6" class="center-text">No points</td></tr>');
                    } else {
    
                        /*  <th>Point Index</th>
                            <th>Point Name</th>
                            <th>Latitude</th>
                            <th>Longitude</th>
                            <th>Point Id</th>
                            <th>Route Id</th> */
    
                        for(var j = 0; j < rte.data.length; j++) {
                            let wpt = rte.data[j]
    
                            let rowID = 'q4-table-row'+i+'-'+j;
                            $('#q4-table').append('<tr id="'+rowID+'"></tr>');
                            
                            $('#'+rowID).append('<td>'+wpt.point_index+'</td>');
                            $('#'+rowID).append('<td>'+(wpt.point_name == null ? '' : wpt.point_name)+'</td>');
                            $('#'+rowID).append('<td>'+wpt.latitude+'</td>');
                            $('#'+rowID).append('<td>'+wpt.longitude+'</td>');
                            $('#'+rowID).append('<td>'+wpt.point_id+'</td>');
                            $('#'+rowID).append('<td>'+wpt.route_id+'</td>');
                        }
                    }
                }
            },
            fail: (err) => {
                alert(err);
                console.log(err);
            },
            error: (err) => {
                alert(err.responseText);
                console.log(err.responseText);
            }
        });
    });

    $('#query5Form').submit((event) => {
        event.preventDefault();
        let type = $('#query5Form input[type="radio"][name="long-or-short"]:checked').val();
        let sortby = $('#query5Form input[type="radio"][name="sort"]:checked').val();
        let filename = getQuery5File();
        let n = $('#n-value').val();
        console.log(filename, n, type);
        jQuery.ajax({
            type: 'get',
            dataType: 'json',
            url: '/query5',
            data: {
                type: type,
                file: filename,
                n_value: n
            },
            success: (data) => {
                if(sortby == 'length') {
                    data.sort((a, b) => (a.route_len > b.route_len) ? 1 : -1);
                } else {
                    data.sort((a, b) => (a.route_name > b.route_name) ? 1 : -1);
                }
                $('#q5-table tr').remove();
                if(data.length == 0) {
                    $('#q5-table').append('<tr class="none"><td colspan="5" class="center-text">No routes</td></tr>');
                }
                $('#q5-table-container').show();
                for(var i = 0; i < data.length; i++) {
                    let rte = data[i];

                    /*  <th>filename</th>
                        <th>route_name</th>
                        <th>route_len</th>  
                        <th>route_id</th>  
                        <th>gpx_id</th>  */

                    let rowID = 'q5-table-row'+i;
                    $('#q5-table').append('<tr id="'+rowID+'"></tr>');

                    $('#'+rowID).append('<td>'+filename+'</td>');
                    $('#'+rowID).append('<td>'+(rte.route_name == null ? '' : rte.route_name)+'</td>');
                    $('#'+rowID).append('<td>'+rte.route_len+'m</td>');
                    $('#'+rowID).append('<td>'+rte.route_id+'</td>');
                    $('#'+rowID).append('<td>'+rte.gpx_id+'</td>');
                }
            },
            fail: (err) => {
                alert(err);
                console.log(err);
            },
            error: (err) => {
                alert(err.responseText);
                console.log(err.responseText);
            }
        });
    });
});

function getGPXViewFile() {
    return $('#gpx-file-picker').val();
}

function getAddRteFile() {
    return $('#add-route-file-picker').val();
}

function getQuery2File() {
    return $('#query2file-picker').val();
}

function getQuery3File() {
    return $('#query3file-picker').val();
}

function getQuery4File() {
    return $('#query4file-picker').val();
}

function getQuery5File() {
    return $('#query5file-picker').val();
}

function getAddWptFile() {
    return $('#add-waypoint-file-picker').val();
}

function getFilesList() {
    let files = [];
    jQuery.ajax({
        type: 'get',
        async: false,
        dataType: 'json',
        url: '/getGPXFiles',
        data: {
            dir: './uploads'
        },
        success: function(data) {
            files = data.files;
        },
        fail: function() {
            return([]);
        }
    });
    return files;
}

function getValidFiles() {
    let files = getFilesList();
    let validFiles = [];
    for(var i = 0; i < files.length; i++) {
        jQuery.ajax({ 
            type: 'get',
            async: false,
            datatype: 'json',
            url: '/validateFile',
            data: {
                file: files[i]
            },
            success: (data) => {
                if(data.value == true) {
                    validFiles.push(files[i]);
                }
            },
            fail: (err) => {
                console.log(err);
                return [];
            },
            error: (err) => {
                console.log(err);
                return [];
            }
        });
    }
    return validFiles;
}

function addToFilelogTable(filename, i) {

    jQuery.ajax({
        type: 'get',
        datatype: 'json',
        url: '/createGPXJSON',
        data: {
            file: filename
        },
        success: function(data) {

            let gpx = JSON.parse(data);
            if($.isEmptyObject(gpx)) {
                alert('failed to upload file');
                console.log('failed to upload file');
                return;
            }

            let rowID = 'file-'+i;
            $('#file-log-table').append('<tr id="'+rowID+'"></tr>');

            $('#'+rowID).append('<td class="filename"><a href="'+filename+'">'+filename+'</a></td>');
            $('#'+rowID).append('<td class="version">'+gpx.version+'</td>');
            $('#'+rowID).append('<td class="creator">'+gpx.creator+'</td>');
            $('#'+rowID).append('<td class="num-points">'+gpx.numWaypoints+'</td>');
            $('#'+rowID).append('<td class="num-routes">'+gpx.numRoutes+'</td>');
            $('#'+rowID).append('<td class="num-tracks">'+gpx.numTracks+'</td>');

            addToFilelistDropdown(filename);
        },
        fail: function(error) {
            alert(error);
            console.log(error);
        }
    });
}

function addToFilelistDropdown(filename) {
    let opt = new Option(filename, filename);
    $(opt).html(filename);
    $('.file-picker').append(opt);
}

function changedGPXFile() {
    let filename = getGPXViewFile();

    $('#gpx-view-table tr').remove();
    $('#rename-input').val('');
    $('#renameForm').hide();

    if(filename == 'default') {
        $('#gpx-view-table').append('<tr><td colspan="6" class="center-text">No file selected</td></tr>');
    } else {
        console.log('changed active GPX file to "' + filename + '"');
        updateGPXPanel(filename);
    }
}

function updateGPXPanel(filename) {

    let found = false;

    jQuery.ajax({
        type: 'get',
        async: false,
        dataType: 'json',
        url: '/createRoutelistJSON',
        data: {
            file: filename
        },
        success: function(data) {
            if(data.length > 0) {
                found = true;
            }
            for(var i = 0; i < data.length; i++) {
                addRoutetoGPXPanel(data[i], i+1);
            }
        },
        fail: function(error) {
            alert(error);
            console.log(error);
        }
    });

    jQuery.ajax({
        type: 'get',
        async: false,
        dataType: 'json',
        url: '/createTracklistJSON',
        data: {
            file: filename
        },
        success: function(data) {
            if(data.length > 0) {
                found = true;
            }
            for(var i = 0; i < data.length; i++) {
                addTracktoGPXPanel(data[i], i+1);
            }
        },
        fail: function(error) {
            alert(error);
            console.log(error);
        }
    });

    if(found == false) {
        $('#gpx-view-table').append('<tr><td colspan="6" class="center-text">No tracks or routes exist in this file</td></tr>');
    }
}

function clickedRow(elmt, id, i) {
    this.event.stopPropagation();
    let name = $('#'+id + ' .name').text();
    let filename = getGPXViewFile();
    
    if($('#'+id).hasClass('track')) {
        jQuery.ajax({
            type: 'get',
            datatype: 'json',
            url: '/getDetailedTrackInfo',
            data: {
                file: filename,
                trackname: name
            },
            success: function(data) {
                let trk = JSON.parse(data);
                if($.isEmptyObject(trk.other)) {
                    alert('no other data exists for this component');
                } else {
                    let str = 'Other Data For Track '+i;
                    for(var o in trk.other) {
                        str += '\n- '+o+': '+trk.other[o];
                    }
                    alert(str);
                }
            },
            fail: function(error) {
                alert(error);
                console.log(error);
            }
        });
    }
    if($('#'+id).hasClass('route')) {
        jQuery.ajax({
            type: 'get',
            datatype: 'json',
            url: '/getDetailedRouteInfo',
            data: {
                file: filename,
                routename: name
            },
            success: function(data) {
                let rte = JSON.parse(data);
                if($.isEmptyObject(rte.other)) {
                    alert('no other data exists for this component');
                } else {
                    let str = 'Other Data For Route '+i;
                    for(var o in rte.other) {
                        str += '\n- '+o+': '+rte.other[o];
                    }
                    alert(str);
                }
            },
            fail: function(error) {
                alert(error);
                console.log(error);
            }
        });
    }
}

function renameComponent(id, i) {
    this.event.stopPropagation();
    $('#rename-input').val('');
    $('#renameForm').show();

    console.log(id, i);

    if($('#'+id).hasClass('renameT-btn')) {
        isTrack = true;
        trackVal = i;
        trackID = 'track-'+i;
        $('#rename-label').text('Rename Track '+i);
    }
    if($('#'+id).hasClass('renameR-btn')) {
        isTrack = false;
        routeVal = i;
        routeID = 'route-'+i;
        $('#rename-label').text('Rename Route '+i);
    }
}

function addTracktoGPXPanel(track, i) {

    let rowID = 'track-'+i;
    $('#gpx-view-table').append('<tr class="track gpx-file-entry" onclick="clickedRow(this, this.id, '+i+')" id="'+rowID+'"></tr>');

    $('#'+rowID).append('<td class="component">Track '+i+'</td>');
    if(track.name === 'None') {
        $('#'+rowID).append('<td class="name"></td>');
    } else {
        $('#'+rowID).append('<td class="name">'+track.name+'</td>');
    }
    $('#'+rowID).append('<td class="num-points">'+track.numPoints+'</td>');
    $('#'+rowID).append('<td class="length">'+track.len+'m</td>');
    $('#'+rowID).append('<td class="loop">'+track.loop+'</td>');
    
    $('#'+rowID).append('<td><div id="renameT-'+i+'" onclick="renameComponent(this.id, '+i+')" class="button renameT-btn">rename</div></td>');
}

function addRoutetoGPXPanel(route, i) {
    
    let rowID = 'route-'+i;
    $('#gpx-view-table').append('<tr class="route gpx-file-entry" onclick="clickedRow(this, this.id, '+i+')" id="'+rowID+'"></tr>');
    
    $('#'+rowID).append('<td class="component">Route '+i+'</td>');
    if(route.name === 'None') {
        $('#'+rowID).append('<td class="name"></td>');
    } else {
        $('#'+rowID).append('<td class="name">'+route.name+'</td>');
    }
    $('#'+rowID).append('<td class="num-points">'+route.numPoints+'</td>');
    $('#'+rowID).append('<td class="length">'+route.len+'m</td>');
    $('#'+rowID).append('<td class="loop">'+route.loop+'</td>');

    $('#'+rowID).append('<td><div id="renameR-'+i+'" onclick="renameComponent(this.id, '+i+')" class="button renameR-btn">rename</div></td>');
}

function changedAddWptFile() {
    let filename = getAddWptFile();
    $('#route-picker option:not(.keep-option)').remove();
    // populate the routes dropdown
    jQuery.ajax({
        type: 'get',
        datatype: 'json',
        url: '/getNumRoutes',
        data: {
            file: filename
        },
        success: function(data) {
            let n = data.value;
            if(n > 0) {
                for(var i = 0; i < n; i++) {
                    let opt = new Option('Route '+(i+1), i+1);
                    $(opt).html('Route '+(i+1));
                    $('#route-picker').append(opt);
                }
            } else {
                alert('this file has no routes\nyou can add a new route or choose a different file')
            }
        },
        fail: function(error) {
            alert(error);
            console.log(error);
        }
    });
}

function changedQuery() {
    $('.queryForm').hide();
    $('.q-table').hide();
    let val = $('#query-selecter').val();
    $('#query'+val+'Form').show();
}

function coninueWithoutLogin() {
    $('#login-background, #loginForm').hide();
    $('#database-stuff').hide();
    $('#login-after-button').show();
    $('#ignoreButton').val('Cancel');
}

function login() {
    $('#login-background, #loginForm').show();
    $('#login-after-button').hide();
}

function storeAllFiles() {
    $('.db-button').prop('disabled', true);
    jQuery.ajax({
        type: 'post',
        url: '/clearData',
        success: (data) => {
            if(data == true) {
                let filenames = getValidFiles();
                jQuery.ajax({
                    type: 'get',
                    dataType: 'json',
                    contentType: 'application/json',
                    url: '/storeAllFiles',
                    data: {
                        files: filenames
                    },
                    success: function(data) {
                        if(data.status == true) {
                            DisplayDBStatus();
                            $('.db-file-picker option:not(.keep-option)').remove();
                            $('#q3-route-picker option:not(.keep-option)').remove();
                            for(var i = 0; i < filenames.length; i++) {
                                addToDBFileList(filenames[i]);
                            }
                        } else {
                            alert('Failed to store all files in the database');
                        }
                        $('.db-button').prop('disabled', false);
                    },
                    fail: function(error) {
                        alert(error);
                        console.log(error);
                        $('.db-button').prop('disabled', false);
                    }
                });
            } else {
                alert('Something went wrong');
                $('.db-button').prop('disabled', false);
            }
        },
        fail: (error) => {
            alert(error);
            console.log(error);
            $('.db-button').prop('disabled', false);
        }
    });
}

function changedQuery3File() {
    let filename = getQuery3File();
    $('#q3-route-picker option:not(.keep-option)').remove();
    // populate the routes dropdown
    jQuery.ajax({
        type: 'get',
        datatype: 'json',
        url: '/getNumRoutes',
        data: {
            file: filename
        },
        success: function(data) {
            let n = data.value;
            console.log(n);
            if(n > 0) {
                for(var i = 0; i < n; i++) {
                    let opt = new Option('Route '+(i+1), i+1);
                    $(opt).html('Route '+(i+1));
                    $('#q3-route-picker').append(opt);
                }
            } else {
                alert('this file has no routes\nyou can add a new route or choose a different file')
            }
        },
        fail: function(error) {
            alert(error);
            console.log(error);
        }
    });
}

function clearAllData() {
    jQuery.ajax({
        type: 'post',
        url: '/clearData',
        success: (data) => {
            if(data == true) {
                DisplayDBStatus();
                $('.db-file-picker option:not(.keep-option)').remove();
                $('#q3-route-picker option:not(.keep-option)').remove();
            } else {
                alert('Failed to clear all data from the database');
            }
        },
        fail: (error) => {
            alert(error);
            console.log(error);
        }
    });
}

function DisplayDBStatus() {
    jQuery.ajax({
        type: 'get',
        url: '/DBstatus',
        success: (data) => {
            console.log(data);
            alert('Database has '+data.files+' files, '+data.routes+' routes, and '+data.points+' points');
        },
        fail: (error) => {
            alert(error);
            console.log(error);
        }
    });
}

function addToDBFileList(filename) {
    let opt = new Option(filename, filename);
    $(opt).html(filename);
    $('.db-file-picker').append(opt);
}