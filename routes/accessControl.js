var models = require('../models');

// POST: /api/check_in
exports.checkIn = function (request, response, next) {
    models.user.findOne({
        where: {
            code: request.body['code'],
            security_token: request.body['security_token']
        }
    })
    .then(data => {
        if (data === null) 
            response.send(401);
        else {
            const user_id = data.get('user_id');
            const user_name = data.get('name');
            models.historic.build({ user_id: user_id, action: 'ENTRADA', date: new Date() }).save();
            response.send(user_name);
        }
    })

    return next();
};

// POST: /api/check_out
exports.checkOut = function (request, response, next) {
   models.user.findOne({
        where: {
            code: request.body['code'],
            security_token: request.body['security_token']
        }
    }).then(user => {
        if (user === null) 
            response.send(401);
        else {
            const user_id = user.get('user_id');
            models.historic.build({ user_id: user_id, action: 'SAIDA', date: new Date() }).save();
            response.send(200);
        }
    })

    return next();
};