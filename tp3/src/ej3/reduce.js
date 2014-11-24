function (key, values) {
    var reducedVal = {count: 0, sum: 0};

    for (var i = 0; i < values.length; i++) {
        reducedVal.count += values[i].count;
        reducedVal.sum += values[i].sum;
    };

    return reducedVal;
};
