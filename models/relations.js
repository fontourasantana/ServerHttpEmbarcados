var models  = require('./')

exports.defineRelations = function(){
    models.user.hasMany(models.historic, { foreignKey: 'user_id', sourceKey: 'user_id' });
}