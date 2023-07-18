/* Requires */
var gulp = require('gulp');
var plumber = require('gulp-plumber');
var concat = require('gulp-concat');
var htmlmin = require('gulp-htmlmin');
var cleancss = require('gulp-clean-css');
var terser = require('gulp-terser');
var gzip = require('gulp-gzip');
var del = require('del');
var markdown = require('gulp-markdown-github-style');
var rename = require('gulp-rename');

/* HTML Task */
gulp.task('html', function() {
    return gulp.src(['html/*.html', 'html/*.htm'])
        .pipe(plumber())
        .pipe(htmlmin({
            collapseWhitespace: true,
            removeComments: true,
            minifyCSS: true,
            minifyJS: true}))
        .pipe(gzip())
        .pipe(gulp.dest('src/data/www'));
});

/* CSS Task */
gulp.task('css', function() {
    return gulp.src(['html/css/*.css'])
        .pipe(plumber())
        .pipe(htmlmin({
            collapseWhitespace: true,
            removeComments: true,
            minifyCSS: true,
            minifyJS: true}))
        .pipe(gzip())
        .pipe(gulp.dest('src/data/www/css'));
});

/* JavaScript Task */
gulp.task('js', function() {
    return gulp.src(['html/js/*.js'])
        .pipe(plumber())
        /* .pipe(terser({ 'toplevel': true }))*/
        .pipe(gzip())
        .pipe(gulp.dest('src/data/www/js'));
});

/* json Task */
gulp.task('json', function() {
    return gulp.src('html/*.json')
        .pipe(plumber())
        .pipe(gzip())
        .pipe(gulp.dest('src/data/www'));
});

/* Image Task */
gulp.task('image', function() {
    return gulp.src(['html/**/*.png', 'html/**/*.ico', 'html/**/*.gif'])
        .pipe(plumber())
        .pipe(gulp.dest('src/data/www'));
});

/* Clean Task */
gulp.task('clean', function() {
    return del(['src/data/www/*']);
});

/* Markdown to HTML Task */
gulp.task('md', function(done) {
    gulp.src('README.md')
        .pipe(plumber())
        .pipe(rename('pixelradio.html'))
        .pipe(markdown())
        .pipe(gulp.dest('dist'));
    gulp.src('Changelog.md')
        .pipe(plumber())
        .pipe(rename('Changelog.html'))
        .pipe(markdown())
        .pipe(gulp.dest('dist'));
    gulp.src('dist/README.md')
        .pipe(plumber())
        .pipe(rename('README.html'))
        .pipe(markdown())
        .pipe(gulp.dest('dist'));
    done();
});

/* CI specific stuff */
gulp.task('ci', function(done) {
    gulp.src(['.ci/warning.md', 'dist/README.md'])
        .pipe(plumber())
        .pipe(concat('README.html'))
        .pipe(markdown())
        .pipe(gulp.dest('dist'));
    done();
});

/* Watch Task */
gulp.task('watch', function() {
    gulp.watch('html/*.html', gulp.series('html'));
    gulp.watch('html/**/*.css', gulp.series('css'));
    gulp.watch('html/**/*.js', gulp.series('js'));
});

/* Default Task */
gulp.task('default', gulp.series(['clean', 'html', 'css', 'js', 'image', 'json']));
