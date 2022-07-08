import module from "module";

const require = module.createRequire(import.meta.url);
const bindings = require("./build/Release/bswap.node");
export default bindings.bswap;
export const native = bindings.bswap;
export const ise = bindings.ise;
export {js} from "./js.mjs";
