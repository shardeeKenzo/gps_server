var PACK_SIZE = 100;

var   packNo = 0
    , packIDDiv
    , track;

function init() {
    if (!gpsdata) return;
    // NOTREACHED
    
    sortData(gpsdata);
    
    console.log(gpsdata.length + " points to process");
    
    track = new L.Polyline([], {color: "#000000", opacity: 1});
    track.addTo(map);
    
    drawTrack(gpsdata, 0, PACK_SIZE);
    
    var goLeft  = document.getElementById("goLeft");
    var goRight = document.getElementById("goRight");
    
    var showAll = document.getElementById("showAll");
    
    goLeft.onclick  = goLeftOnClick;
    goRight.onclick = goRightOnClick;
    
    showAll.onclick = showAllPoints;
    
    packIDDiv  = document.getElementById("packID");
}

function drawTrack(aData, start, finish) {
    var   i, len = aData.length
        , points = [];
    
    if (len < finish) finish = len;
    
    for (i = start; i < finish; i++) {
        points.push({lat: aData[i].x, lon: aData[i].y});
    }
    
    track.setLatLngs(points);
    track.redraw();
    //var track = new L.Polyline(points, {color: "#000000", opacity: 1});
    //track.addTo(map);
}

function goRightOnClick() {
    if (gpsdata.length <= packNo * PACK_SIZE)  {
        packNo = 0;
    } else {
        packNo++;
    }
    
    var finish = (packNo + 1) * PACK_SIZE;
    
    finish = (finish < gpsdata.length) ? finish : gpsdata.length - 1;
    
    drawTrack(gpsdata, packNo * PACK_SIZE, (packNo + 1) * PACK_SIZE);
    
    map.panTo({lat: gpsdata[finish].x, lon: gpsdata[finish].y});
    
    packIDDiv.innerHTML = "pack: " + packNo;
}

function goLeftOnClick() {
    if (0 === packNo)  {
        packNo = Math.floor(gpsdata.length / PACK_SIZE);
    } else {
        packNo--;
    }
    
    var finish = (packNo + 1) * PACK_SIZE;
    
    drawTrack(gpsdata, packNo * PACK_SIZE, (packNo + 1) * PACK_SIZE);
    
    map.panTo({lat: gpsdata[finish].x, lon: gpsdata[finish].y});
    
    packIDDiv.innerHTML = "pack: " + packNo; 
}

function showAllPoints() {
    drawTrack(gpsdata, 0, gpsdata.length - 1);
}

function sortData(anArray) {
    anArray.sort(function(a,b){return a.t - b.t});
    
    //~ var i, j, len, f, min;
    //~ 
    //~ len = anArray.length;
    
    //~ FOR J=1 TO N-1 STEP 1
    //~ F=0
    //~ MIN=J
    //~ FOR I=J TO N-J STEP 1 
       //~ IF Y[I]>Y[I+1] THEN SWAP Y[I],Y[I+1]:F=1
       //~ IF Y[I]<Y[MIN] THEN MIN=I
       //~ NEXT I
    //~ IF F=0 THEN EXIT FOR
    //~ IF MIN<>J THEN SWAP Y[J],Y[MIN]
    //~ NEXT J
    
    //~ for (i = 0; i < len - 1; i++) {
        //~ f = 0;
        //~ min = i;
        //~ for (j = i; j < len - (i + 1); j++) {
            //~ if (anArray[j].t > anArray[j + 1].t) {
                //~ //console.log("sorted!");
                //~ anArray[j] = [anArray[j + 1], anArray[j + 1] = anArray[j]][0];
                //~ f = 1;
            //~ }
            //~ if (anArray[j].t < anArray[min].t) min = j;
        //~ }
        //~ if (!f) break;
        //~ 
        //~ if (min !== i) anArray[i] = [anArray[min], anArray[min] = anArray[i]][0];
    //~ }
}

//
//
//
