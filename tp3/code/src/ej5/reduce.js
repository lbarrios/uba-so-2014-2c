function (key, values) {
    var res = {score: 0, cantPalabras: 0};
    for (var i = 0; i < values.length; i++) {
        var val = values[i];
        res.score += val.score;
        res.cantPalabras += val.cantPalabras;
    }
    return res;
}
