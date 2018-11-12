module.exports = function (sequelize, DataTypes) {
    var User = sequelize.define('user', {
        user_id: {
            type: DataTypes.INTEGER.UNSIGNED,
            autoIncrement: true,
            primaryKey: true
        },
        code: {
            type: DataTypes.STRING,
            unique: true
        },
        name: DataTypes.STRING,
        security_token: {
            type: DataTypes.STRING,
            unique: true
        }
    });

    return User;
};