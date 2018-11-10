const restify = require('restify')
const Sequelize = require('sequelize')
const sequelize = new Sequelize({
    database: 'embarcados',
    username: 'root',
    password: null,
    dialect: 'mysql',
    operatorsAliases: false
});
const servidor = restify.createServer()
const porta = 8080;

const User = sequelize.define('user', {
    user_id: {
        type: Sequelize.INTEGER.UNSIGNED,
        autoIncrement: true,
        primaryKey: true
    },
    code: Sequelize.STRING,
    name: Sequelize.STRING,
    security_token: Sequelize.STRING
})

const Historic = sequelize.define('historic', {
    historic_id: {
        type: Sequelize.INTEGER.UNSIGNED,
        autoIncrement: true,
        primaryKey: true
    },
    user_id: {
        type: Sequelize.INTEGER.UNSIGNED,
        allowNull: false,
    },
    action: {
        type: Sequelize.STRING(7),
        allowNull: false
    },
    date: Sequelize.DATE
}, { timestamps: false })

servidor.get('/check_in/:code/:security_token', (request, response, next) => {
    User.findOne({
        where: {
            code: request.params['code'],
            security_token: request.params['security_token']
        }
    }).then(user => {
        if (user === null)
            response.send(401);

        const user_id = user.get('user_id');
        const user_name = user.get('name');
        Historic.build({ user_id: user_id, action: 'ENTRADA', date: new Date() }).save();
        response.send({
            user_id: user_id,
            name: user_name
        });
    })

    return next();
})

servidor.get('/check_out/:code/:security_token', (request, response, next) => {
    User.findOne({
        where: {
            code: request.params['code'],
            security_token: request.params['security_token']
        }
    }).then(user => {
        if (user === null) response.send(401);

        const user_id = user.get('user_id');
        Historic.build({ user_id: user_id, action: 'SAIDA', date: new Date() }).save();
        response.send(200);
    })

    return next();
})

servidor.listen(porta, () => {
    console.log(`Servidor online em http://localhost:${porta}`)
})

User.sync({ force: true });
Historic.sync({ force: true });
User.hasMany(Historic, { foreignKey: 'user_id', sourceKey: 'user_id' });

/*User.create({
    code: '123',
    name: 'Professor 1',
    security_token: '918237m12387da6sd876xcz765123*!SDSxasd1'
});
User.create({
    code: '456',
    name: 'Professor 2',
    security_token: '918237m12387da6sd876xcz765123*!SDSxasd2'
});
User.create({
    code: '789',
    name: 'Professor 3',
    security_token: '918237m12387da6sd876xcz765123*!SDSxasd3'
});*/