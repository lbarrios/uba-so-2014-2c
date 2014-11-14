/**
 * Ejercicio 1: Encontrar el subreddit con mayor score promedio.
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
db.ej1.find().sort({"value.sum": -1}).limit(1);

/**
 * Ejercicio 2: Encontrar los doce títulos con mayor score 
 * de la colección de posts con al menos 2000 votos.
 */
use reddit;
db.ej2.remove({});
db.posts.find(
    {total_votes:{$gt:2000}},
    {title:1}
).sort({
    "score":-1
}).limit(12);

/**
 * Ejercicio 3: Para los diez mejores scores, 
 * calcular la cantidad de comentarios en promedio por sumisión.
 */
use reddit;
db.ej3.remove({});
var ej3_cursor = db.posts.find().sort({
    "score":-1
}).limit(10);
var total = 0;
while(ej3_cursor.hasNext()) {
    var one_post = ej3_cursor.next();
    total += one_post.number_of_comments;
}
var promedio = total/10;
print(promedio);
/*
var m = function() {
  emit(0, this.number_of_comments);
}
var r = function(key, values) {
  return Array.sum(values);
}
db.posts.mapReduce(m,r,{out:"ej3",sort:{"score":-1},limit:10});
*/

/**
 * Ejercicio 4: Entre los usuarios con a la sumo 5 sumisiones, 
 * encontrar el que posea mayor cantidad de upvotes.
 */
use reddit;
db.ej4.remove({});
var m = function() {
  emit(this.username, {count: 1, upvotes: this.number_of_upvotes});
};
var r = function(key, values) {
  reducedVal = {count: 0, upvotes: 0};

  for (var i = 0; i < values.length; i++) {
    reducedVal.count += values[i].count;
    reducedVal.upvotes += values[i].upvotes;
  };

  return reducedVal;
};
db.posts.mapReduce(m,r,{
  out: "ej4", 
});
db.ej4.find({"value.count":{$lte:5}}).sort({"value.upvotes": -1}).limit(1);

/**
 * Ejercicio 5: Para todos los subrredit que poseen un score ente 280 y 300, 
 * indicar la cantidad palabras presentes en sus títulos
 */

// No entendí el ejercicio... consultar
