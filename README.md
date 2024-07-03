# WebCXX
WebCXX is an in-development modern C++ web back-end built upon CgiCC which expands on many of the missing features, while providing a more robust framework containing all the features requiried to be actually usable on a production website.
The purpose of this project is to provide a highly performant and optimized back-end framework for high-traffic applications that are willing to trade compilation time for better performance and lower resource overhead.

## Features and roadmap
- [x] JSON parsing
- [x] Handle `application/json` POST requests correctly
- [x] Connect to external REST APIs
- [x] Routing, including routes with arguments
- [ ] User-friendly front-end language (similar to Blazor)
- [x] Route verbs
- [ ] XML parsing
- [ ] Database drivers
- [ ] Test framework

## Database status
Supported databases:
- [x] MySQL

Supported features:
- [x] Models with serialization and deserialization
- [x] Insert/update/delete row
- [ ] Create/update/delete table
- [ ] Create/delete database
- [ ] Seed database
- [ ] Relationships

## Tests status
![Build status with MySQL](https://github.com/devcore96/webcxx/actions/workflows/build-mysql.yml/badge.svg)
![Tests status with MySQL](https://github.com/devcore96/webcxx/actions/workflows/run-tests-mysql.yaml/badge.svg)

![Build status without DB](https://github.com/devcore96/webcxx/actions/workflows/build-no-db.yml/badge.svg)
![Tests status without DB](https://github.com/devcore96/webcxx/actions/workflows/run-tests-no-db.yaml/badge.svg)
