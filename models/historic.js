module.exports = function(sequelize, DataTypes) {
	var Historic = sequelize.define('historic', {
		historic_id: {
            type: DataTypes.INTEGER.UNSIGNED,
            autoIncrement: true,
            primaryKey: true
        },
        user_id: {
            type: DataTypes.INTEGER.UNSIGNED,
            allowNull: false,
        },
        action: {
            type: DataTypes.STRING(7),
            allowNull: false
        },
        date: DataTypes.DATE
	}, { timestamps: false });

	return Historic;
};