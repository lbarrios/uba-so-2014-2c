function () {
    var cantPalabras = this.title.split(' ').length;
    emit(this.subreddit, {score: this.score, cantPalabras: cantPalabras});
}
