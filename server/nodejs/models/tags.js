const db = require('./db');

const table = 'tags';

const all = () => {
  return db.all(table);
};

const find = (id) => {
  return db.find(table, id);
};

const searchByTag = (tag) => {
  return db.query(`SELECT * FROM ${table} WHERE tag = '${tag}'`)
};

const create = (data) => {
  const query = `INSERT INTO ${table} (id_user, tag) VALUES ('${data.id_user}', '${data.tag}');`;

  return db.query(query);
};

const update = (id, data) => {
  const query = `UPDATE ${table} SET tag = '${data.tag}', id_user = '${data.id_user}', state = ${data.state} WHERE id = ${id};`;

  return db.query(query);
};

const remove = (id) => {
  return db.remove(table, id);
};

module.exports = {
  all: all,
  find: find,
  searchByTag: searchByTag,
  create: create,
  update: update,
  remove: remove
};
