UNSELECTED_ICON_SIZE           = [16, 16];
SELECTED_ICON_SIZE             = [20, 20];
FOCUSED_ICON_SIZE             = [25, 25];

USEL_ICON = L.divIcon(
    {
        className   :   "unselectedMarker",
        iconSize    :   UNSELECTED_ICON_SIZE,
        html        :   " "
    }
);

SEL_ICON = L.divIcon(
    {
        className   :   "selectedMarker",
        iconSize    :   SELECTED_ICON_SIZE,
        html        :   " "
    }
);

FOC_ICON = L.divIcon(
    {
        className   :   "focusedMarker",
        iconSize    :   FOCUSED_ICON_SIZE,
        html        :   " "
    }
);

DASH_ARRAY                    = "3, 5, 3, 5, 15, 10";

function isEmpty(str) {
    return (!str || 0 === str.length);
}

function renderTime(val) {
    var mins = 0;
    var hrs = 0;
    var buf = val;
    while (60 < buf) {
        mins++;
        buf -= 60;
        if (60 == mins) {
            mins = 0;
            hrs++;
        }
    }
    if (hrs  < 10) hrs  = "0" + hrs;
    if (mins < 10) mins = "0" + mins;
        
    return hrs + ":" + mins; 
}

function renderTimeUNIX(val) {
    var   date  = new Date(val * 1000)
        , hr   = date.getHours()
        , min  = date.getMinutes();
        
    if (hr < 10)  hr  = "0" + hr;
    if (min < 10) min = "0" + min;
        
    return hr + ":" + min; 
}

function makeRecordsForInfoTable() {
    // creating content for table
    var   curRoute = dataOut.routes[pathID]
        , path     = curRoute.deliveries
        , len      = path.length
        , i
        , job
        , jobID
        , trID
        , tr
        , route   = {
             weight  : 0
           , volume  : 0
           , price   : 0
           , time    : 0
       }
       , car     = {
             weight  : 0
           , volume  : 0
           , price   : 0
           , time    : 0
       }
       
    route.jobCnt = 0;
    for (i = 0; i < len; i++) {
        // jobs are filled outside in pathVis.js:init()
        job = jobs[ path[i].jobID ];
        
        if (undefined == job) continue;
        
        route.weight += job.weigth;
        route.volume += job.volume;
        route.price  += job.value;
        
        route.jobCnt++;
    }
    
    route.time = path[len - 1].arrival - path[0].arrival;
    route.time = renderTime(route.time);
    
    route.value = dataOut.routes[pathID].value;
    
    route.dist          = (curRoute.totalDistance / 1000) + "км";
    //route.jobCnt        = len;
    
    jobID = taskID;
    
    trID = dataOut.routes[pathID].transportID;
    tr   = cars[trID];
    
    car.weight = tr.maxweigth;
    car.volume = tr.maxvolume;
    car.price  = tr.maxvalue;
    
    car.time   = renderTime(tr.time_max);

    var records =  [
        { recid: 1, caption: "вес:  "           , route: route.weight , car: car.weight},
        { recid: 2, caption: "объём:"           , route: route.volume , car: car.volume},
        { recid: 3, caption: "цена: "           , route: route.price  , car: car.price},
        { recid: 4, caption: "время:"           , route: route.time   , car: car.time},
        { recid: 5, caption: "общее расстояние:", route: route.dist   , car: "--"},
        { recid: 6, caption: "стоимость:"       , route: route.value  , car: "--"},
        { recid: 7, caption: "Кол-во точек:"    , route: route.jobCnt , car: "--"},
    ];
    
    return records;
}

function makeRecordsForRouteTable() {
    // creating content for table
    var   curRoute = dataOut.routes[pathID]
        , path     = curRoute.deliveries
        , len      = path.length
        , i
        , j
        , el
        , weight
        , volume
        , wins
        , windows 
        , records  = []
        , point;
       
    for (i = 0; i < len; i++) {
        el     = path[i];
        weight = (jobs[el.jobID]) ? jobs[el.jobID].weigth : 0;
        volume = (jobs[el.jobID]) ? jobs[el.jobID].volume : 0;
        wins   = (jobs[el.jobID]) ? jobs[el.jobID].windows : [];
        
        windows = "";
        for (j = 0; j < wins.length; j++) {
            if (0 != j) windows += "+ ";
            
            from = renderTimeUNIX(wins[j].start);
            to   = renderTimeUNIX(wins[j].finish);
            windows += from + ":" + to;
        }
        
        if (undefined == el) continue;
        
        records.push(
            {
                  recid       : i + 1
                , jobID       : el.jobID
                , windows     : windows
                , arrival     : renderTimeUNIX(el.arrival)
                , distance    : (el.distance / 1000) + "км"
                , duration    : (el.duration / 60).toFixed(2) + "мин"
                , downtime    : (el.downtime / 60).toFixed(2) + "мин"
                , servicetime : (el.servicetime / 60) + "мин"
                , weight      : weight
                , volume      : volume
            }
        );
    }
    
    records.push(
        {
              summary     : true
            , recid       : "S-1"
            , windows     : ""
            , arrival     : ""
            , distance    : "<span style='font-weight: bold;'>" + (curRoute.totalDistance / 1000) + "км </span>"
            , duration    : "<span style='font-weight: bold;'>" + (curRoute.totalDuration / 3600).toFixed(2) + "ч </span>"
            , downtime    : "<span style='font-weight: bold;'>" + (curRoute.totalDowntime / 3600).toFixed(2) + "ч </span>"
            , servicetime : ""
        }
    );
    
    return records;
}

function toggleSideBarEvent (e) {
    var target = (e.target instanceof L.Marker) ? e.target : e.layer;

    toggleSideBar();
}

function toggleSideBar () {
    if (undefined == document.getElementById('infoTable')) {
        var   infoRecords  = makeRecordsForInfoTable()
            , routeRecords = makeRecordsForRouteTable();
            
        w2ui['layout'].toggle('right');
        
        w2ui['layout'].content(
              'right'
            , '<div id="infoTable" style="height: 40%"></div>'
            + '<div id="routeTable" style="height: 60%"></div>'
        );
        w2ui['layout'].refresh('right');
        
        $('#infoTable').ready(
            function () {
                infoTable.create(infoRecords);
            }
        );
        
        $('#routeTable').ready(
            function () {
                routeTable.create(routeRecords);
            }
        );
    } else {
        w2ui['layout'].toggle('right');
        
        // DEBUG
        console.log('destroying infoTable');
        
        $().w2destroy('infoTable');
        $().w2destroy('routeTable');
        
        var el = document.getElementById('infoTable');
        el.parentNode.removeChild(el);
    }
}

infoTable = {
    create: function(records) {
        $('#infoTable').w2grid({ 
            name: 'infoTable', 
            columns: [
                { field: 'caption', caption: '#      ', size: '20%' },              
                { field: 'route'  , caption: 'Маршрут', size: '25%' },
                { field: 'car'    , caption: 'Машина' , size: '55%' }
            ],
            records: records
        });
    },
        
    fill: function(records) {
        var   i   = 0 
            , len = records.length
            , rec;
            
        if (!w2ui['infoTable']) return;
        // NOTREACHED
        
        w2ui['infoTable'].clear();
        
        w2ui['infoTable'].add(records);
        //for (i = 0; i < len; i++) {
        //    rec = records[i];
        //    w2ui['infoTable'].add(rec);
        //}
        
    }
}

routeTable = {
    create: function(records) {
        $('#routeTable').w2grid({
            show      : {lineNumbers: true},
            name      : 'routeTable',
            multiSort : false,
            sortOnAdd : false,
            columns: [
                  { field: 'jobID'       , caption: 'jobID'      , size: '10%', sortable: false }
                , { field: 'windows'     , caption: 'windows'    , size: '15%', sortable: false }
                , { field: 'arrival'     , caption: 'arrival'    , size: '10%', sortable: false }
                , { field: 'distance'    , caption: 'distance'   , size: '10%', sortable: false }
                , { field: 'duration'    , caption: 'duration'   , size: '10%', sortable: false }
                , { field: 'downtime'    , caption: 'downtime'   , size: '10%', sortable: false }
                , { field: 'servicetime' , caption: 'servicetime', size: '11%', sortable: false }
                , { field: 'weight'      , caption: 'weight'     , size: '10%', sortable: false }
                , { field: 'volume'      , caption: 'volume'     , size: '14%', sortable: false }
            ]
            , records: records
            , onSelect: function (target, eventData) {
                var   recid = eventData.recid
                    , jobID = records[recid - 1].jobID;
                    
                if (-1 == jobID) return;
                // NOTREACHED
                
                var focIcon = L.divIcon(
                    {
                        className   :   "focusedMarker",
                        iconSize    :   FOCUSED_ICON_SIZE,
                        html        :   jobID + "",
                        zIndexOffset:   2000
                    }
                );
                
                if (routeTable._prevFocusedMarker) {
                    var m = routeTable._prevFocusedMarker;
                    var oldIcon = L.divIcon(
                        {
                              className    : m._className
                            , iconSize     : m._iconSize
                            , html         : m._jobID
                            , zIndexOffset : m._zIndexOffset
                        }
                    );
                    $(m._icon).css("background", m._backround);
                    m.setIcon(oldIcon);
                }
                
                if (0 < jobs[jobID].taskID) {
                    taskID = jobs[jobID].taskID - 1;
                    highlightTaskRoute(taskID);
                    taskIDDiv.innerHTML = "taskID: " + taskID; 
                }
                
                routeTable._prevFocusedMarker = markers[jobID];
                
                markers[jobID].setIcon(focIcon);
                
                map.panTo(markers[jobID]._latlng);
            }
        });
    },
        
    fill: function(records) {
        var   i   = 0 
            , len = records.length
            , rec;
            
        if (!w2ui['routeTable']) return;
        // NOTREACHED
        
        w2ui['routeTable'].clear();
        w2ui['routeTable'].add(records);
        //for (i = 0; i < len; i++) {
        //    rec = records[i];
        //    w2ui['routeTable'].add(rec);
        //}
    }
}

/*
 * 
 */