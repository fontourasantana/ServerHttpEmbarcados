var fs = require('fs');
var path = require('path');
var Sequelize = require('sequelize');
var lodash = require('lodash');
var sequelize = new Sequelize({
    database: 'embarcados',
    username: 'root',
    password: null,
    dialect: 'mysql',
    operatorsAliases: false
});
var db = {};

fs
    .readdirSync(__dirname)
    .filter(function (file) {
        return ((file.indexOf('.') !== 0) && (file !== 'index.js') && (file !== 'relations.js') && (file.slice(-3) === '.js'));
    })
    .forEach(function (file) {
        var model = sequelize.import(path.join(__dirname, file));
        db[model.name] = model;
    });

Object.keys(db).forEach(function (modelName) {
    if (db[modelName].options.hasOwnProperty('associate')) db[modelName].options.associate(db);
});

module.exports = lodash.extend({
    sequelize: sequelize,
    Sequelize: Sequelize
}, db);