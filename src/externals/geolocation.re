type coords = {
  .
  "latitude": float,
  "longitude": float
};

type position = {
  .
  "coords": coords
  /* not using timestamp */
};

type error = {
  .
  code: int,
  message: string
};

type options = {
  enableHighAccuracy: bool,
  timeout: int,
  maximumAge: int
};

[@bs.val] [@bs.scope ("navigator", "geolocation")]
external getCurrentPosition_ :
  (position => unit, error => unit, options) => unit =
  "getCurrentPosition";

let getCurrentPosition = (~success, ~error, options: options) =>
  getCurrentPosition_(success, error, options);

getCurrentPosition;