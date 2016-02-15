 Date.prototype.yyyymmdd = function() {
   var yyyy = this.getFullYear().toString();
   var mm = (this.getMonth()+1).toString(); // getMonth() is zero-based
   var dd  = this.getDate().toString();
   return yyyy + '-' + (mm[1]?mm:"0"+mm[0]) + '-' + (dd[1]?dd:"0"+dd[0]); // padding
  };

var d = new Date();
var today = d.yyyymmdd();

var opts = {
  url: 'https://statsapi.web.nhl.com/api/v1/schedule?startDate=' + today +
    '&endDate=' + today + '&expand=schedule.teams,schedule.linescore,schedule.broadcasts,schedule.ticket,schedule.game.content.media.epg&leaderCategories=&site=en_nhlCA&teamId=',
  type: 'json'
};

function ajaxCall() {
  var req = new XMLHttpRequest();
  
  req.open('GET', opts.url);
  req.onload = function(e) {
    if (req.readyState === 4 && req.status === 200) {
      var data = JSON.parse(req.responseText);
      Pebble.sendAppMessage({ '0': getScore(data), });
    }
  }
  
  req.send(null);
}

function getScore(data) {
  var score = '';
  
  data.dates.map(function (date) {
    // date === [ { date: '', games: [...]  }]
    
    date.games.map(function (game) {
      // da leafsz === 10
      if (game.teams.away.team.id === 10) {
        score = game.teams.away.score + ' - ' + game.teams.home.score;
      } else if (game.teams.home.team.id === 10) {
        score = game.teams.home.score + ' - ' + game.teams.away.score;
      }
    });
  });
  
  return score;
}
				
Pebble.addEventListener("ready", function(e)  { console.log("Pebble :: ready!") });
Pebble.addEventListener("appmessage", function(e) { ajaxCall(); });