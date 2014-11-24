function finalize (key, reducedVal) {
    return {
        sum: reducedVal.sum,
        count: reducedVal.count,
        prom: reducedVal.sum/reducedVal.count
    };
}
