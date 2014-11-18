function (key, values) {
    var res = {sumisiones: 0, number_of_upvotes: 0};
    for (var i = 0; i < values.length; i++) {
        var val = values[i];
        res.sumisiones += val.sumisiones;
        res.number_of_upvotes += val.number_of_upvotes;
    }
    return res;
}
