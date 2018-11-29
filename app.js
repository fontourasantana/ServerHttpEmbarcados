var restify = require('restify')
var db = require('./models')
const port = 8080;
var server = restify.createServer({ name: 'Servidor HTTP - Trabalho Embarcados' })
var routes = require('./routes');
var relations = require('./models/relations')

server.use(restify.plugins.bodyParser());
server.post('/api/check_in/', routes.accessControl.checkIn);
server.post('/api/check_out/', routes.accessControl.checkOut);

const seedUsers = function () {
    db.user.create({
        code: '123',
        name: 'Professor Isaque',
        security_token: '918237m12387da6sd876xcz765123*!SDSxasd1'
    });
    db.user.create({
        code: '456',
        name: 'Professor Platao',
        security_token: '918237m12387da6sd876xcz765123*!SDSxasd2'
    });
    db.user.create({
        code: '789',
        name: 'Professor Newton',
        security_token: '918237m12387da6sd876xcz765123*!SDSxasd3'
    });
}

relations.defineRelations();

db
    .sequelize
    .sync({ force: false })
    // .then(seedUsers)
    .done(callback => {
        server.listen(port);
        console.log(`Servidor online em http://localhost:${port}`)
    })