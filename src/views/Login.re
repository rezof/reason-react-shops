let component = ReasonReact.statelessComponent("Login");

let make = _children => {
  ...component,
  render: _self =>
    <div className="">
      <div className="form-wrapper">
        <form>
          <div> <input _type="email" /> </div>
          <div> <input _type="password" /> </div>
          <div> <input _type="submit" value="Login" /> </div>
        </form>
      </div>
    </div>
};