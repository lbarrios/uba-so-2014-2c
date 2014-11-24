function () {
    if (this.total_votes >= 2000)
        emit(this.title, this.total_votes);
}
