// One AppMessage at a time: a second one sent while the first is in flight
// fails, and weather and every API complication send on their own timers.

var queue = [], busy = false;

function next() {
  var job = queue.shift();
  busy = !!job;
  if (!job) { return; }
  Pebble.sendAppMessage(job.msg, function() {
    next();
    if (job.ok) { job.ok(); }
  }, function(err) {
    next();
    if (job.fail) { job.fail(err); }
  });
}

module.exports = function(msg, ok, fail) {
  queue.push({ msg: msg, ok: ok, fail: fail });
  if (!busy) { next(); }
};
