/**
 * Ejercicio 1
 */
use reddit;
db.ej1.remove({});
var m = function() { 
	emit(this.subreddit, { count:1, sum:this.score} ); 
};
var r = function(key, values) { 
	reducedVal = {count: 0, sum: 0};

	for (var i = 0; i < values.length; i++) {
		reducedVal.count += values[i].count;
		reducedVal.sum += values[i].sum;
	};
	
	return reducedVal;
};
var f = function(key, reducedVal) { return {sum: reducedVal.sum, count: reducedVal.count, prom: reducedVal.sum/reducedVal.count} ; };
db.posts.mapReduce(m,r,{ out: "ej1", finalize: f});
db.ej1.find().sort({"value.sum": -1}).limit(1)


/**
 * Ejercicio 2
 */
use reddit;
db.ej2.remove({});
db.posts.find({total_votes:{$gt:2000}}).sort({"score":-1}).limit(12);

/**
 * Ejercicio 3
 */
use reddit;
db.ej3.remove({});
var m = function() {
	emit(0, this.number_of_comments);
}
var r = function(key, values) {
	return Array.sum(values);
}
//db.posts.find().sort({"score":-1}).limit(10);
db.posts.mapReduce(m,r,{out:"ej3",sort:{"score":1},limit:10});
//db.posts.mapReduce(m,r,{out:"ej3",sort:{"score":-1},limit:10});

db.runCommand({mapReduce: db.posts,
                 map: m,
                 out: { merge: out },);